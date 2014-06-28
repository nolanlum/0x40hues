#include <assert.h>

#include <common.hpp>
#include <video_renderer.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// All GLUT callbacks are contained in the HuesRenderer namespace to avoid polluting the global
// namespace with "private" functions. It's gross.
namespace HuesRenderer {

  void DrawFrameCallback() {
    ::VideoRenderer::instance->DrawFrame();
  }

  void ResizeCallback(const int width, const int height) {
    ::VideoRenderer::instance->HandleResize(width, height);
  }

  void TimerCallback(int ignored) {
    glutPostRedisplay();

    // Cap at 500fps, I guess.
    glutTimerFunc(2, TimerCallback, 0);
  }

}

const char *VideoRenderer::kPassThroughVertexShader = "void main() { gl_Position = ftransform(); }";
const char *VideoRenderer::kHardLightFragmentShader = R"END(
uniform sampler2D BaseImage;
uniform vec4 BlendColor;

void main() {
  vec4 base = texture2D(BaseImage, gl_FragCoord);
  vec4 lumCoeff = vec4(0.2125, 0.7154, 0.0721, 1.0);

  float luminance = dot(BlendColor, lumCoeff);
  if (luminance < 0.45) {
    gl_FragColor = 2.0 * BlendColor * base;
  } else if (luminance > 0.55) {
    gl_FragColor = vec4(1.0) - 2.0 * (vec4(1.0) - BlendColor) * (vec4(1.0) - base);
  } else {
    vec4 result1 = 2.0 * BlendColor * base;
    vec4 result2 = vec4(1.0) - 2.0 * (vec4(1.0) - BlendColor) * (vec4(1.0) - base);
    gl_FragColor = mix(result1, result2, (luminance - 0.45) * 10.0);
  }

  gl_FragColor = base;
}
)END";

VideoRenderer *VideoRenderer::instance = NULL;

VideoRenderer::VideoRenderer() {
  pthread_rwlock_init(&this->render_lock, NULL);
}
VideoRenderer::~VideoRenderer() {}

void VideoRenderer::Init(int argc, char *argv[]) {
  // Set preferred window size and location.
  glutInitWindowSize(1280, 720);
  glutInitWindowPosition(50, 50);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInit(&argc, argv);

  // Create the OpenGL context and window.
  glutCreateWindow("0x40 Hues");

  // Initialize GLEW.
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    ERR("GLEW initialization failed! Failure reason on next line.");
    ERR(glewGetErrorString(err));
    exit(EXIT_FAILURE);
  }

  // Compile shaders.
  this->CompileShaders();

  // Set callback functions.
  glutDisplayFunc(HuesRenderer::DrawFrameCallback);
  glutReshapeFunc(HuesRenderer::ResizeCallback);
  glutTimerFunc(0, HuesRenderer::TimerCallback, 0);

  // Clear the window and display a solid color.
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();

  glUseProgram(this->shader_program);
}

void VideoRenderer::DoGlutLoop() {
  // Refuse to run if we're not the One True Singleton.
  if (VideoRenderer::instance != NULL) {
    return;
  }
  VideoRenderer::instance = this;

  // Actually call the main loop.
  glutMainLoop();

  // I don't know how we can ever get here but whatever.
  VideoRenderer::instance = NULL;
}

void VideoRenderer::CompileShaders() {
  // Compile shaders and link program.
  this->pass_through_vertex_shader =
      this->CompileShader(VideoRenderer::kPassThroughVertexShader, GL_VERTEX_SHADER);
  this->hard_light_fragment_shader =
      this->CompileShader(VideoRenderer::kHardLightFragmentShader, GL_FRAGMENT_SHADER);

  this->shader_program = glCreateProgram();
  glAttachShader(this->shader_program, this->pass_through_vertex_shader);
  glAttachShader(this->shader_program, this->hard_light_fragment_shader);
  glLinkProgram(this->shader_program);
}

GLuint VideoRenderer::CompileShader(const char *&shader_text, GLenum shader_type) {
  GLuint shader;
  GLint shaderOpSuccess;

  shader = glCreateShader(shader_type);
  glShaderSourceARB(shader, 1, &shader_text, NULL);
  glCompileShaderARB(shader);
  glGetObjectParameterivARB(shader, GL_COMPILE_STATUS, &shaderOpSuccess);
  if (!shaderOpSuccess) {
    GLint log_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 1) {
      GLchar* log = reinterpret_cast<GLchar*>(malloc(log_len));
      glGetInfoLogARB(shader, log_len, NULL, log);
      ERR("Error compiling shader! Output on next lines.");
      ERR(log);
      free(log);
    }
  }

  return shader;
}

void VideoRenderer::LoadTextures(const ResourcePack &respack) {
  vector<ImageResource*> images;
  respack.GetAllImages(images);

  for(ImageResource *image : images) {
    int width;
    int height;
    int color_type;

    DEBUG("Loading [" + image->GetName() + "] into texture memory.");
    png_byte *image_bytes = image->ReadAndDecode(&width, &height, &color_type);

    GLint texture_format;
    switch(color_type) {
      case PNG_COLOR_TYPE_RGB_ALPHA:
        texture_format = GL_RGBA;
        break;
      default:
        ERR("Unsupported libpng color type: [" + to_string(color_type) + "].");
        free(image_bytes);
        return;
    }

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, /* level of detail number */0, GL_LUMINANCE_ALPHA,
        width, height, /* border */ 0, texture_format, GL_UNSIGNED_BYTE, image_bytes);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, (GLuint) NULL);

    free(image_bytes);
    this->images[image->GetName()] = image;
    this->textures[image->GetName()] = texture;
  }
}

bool VideoRenderer::SetImage(const string& image_name, const AudioResource::Beat transition) {
  pthread_rwlock_wrlock(&this->render_lock);

  if (this->images.find(image_name) == this->images.end()) {
    ERR("Render image [" + image_name + "] failed: not loaded!");
    pthread_rwlock_unlock(&this->render_lock);
    return false;
  }

  this->current_image = this->images[image_name];
  glutPostRedisplay();

  pthread_rwlock_unlock(&this->render_lock);

  LOG("Starting transition to [" + image_name + "].");
  return true;
}

bool VideoRenderer::SetColor(const int color_index) {
  this->current_color = (color_index < 0) ? 0 : (color_index % 40);
  return true;
}

void VideoRenderer::DrawFrame() {
  pthread_rwlock_rdlock(&this->render_lock);

  // Calculate the color based on the index, normalized to [0,1].
  float red = (this->current_color % 4) / 3.f;
  float green = ((this->current_color >> 2) % 4) / 3.f;
  float blue = (this->current_color >> 4) / 3.f;
  glClearColor(red, green, blue, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  //DEBUG(to_string(red) + " " + to_string(green) + " " + to_string(blue));

  // Make sure we have a image to draw, first.
  if (this->current_image) {
    // Set shader program uniforms.
    GLuint texture = this->textures[this->current_image->GetName()];
    glUniform1i(glGetUniformLocation(this->shader_program, "BlendImage"), 0);
    glUniform4f(glGetUniformLocation(this->shader_program, "BaseColor"), red, green, blue, 1);

    // Bind the texture and draw a rectangle.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(0, 0);
        glTexCoord2f(1, 1); glVertex2f(this->window_width, 0);
        glTexCoord2f(1, 0); glVertex2f(this->window_width, this->window_height);
        glTexCoord2f(0, 0); glVertex2f(0, this->window_height);
    glEnd();
  }

  // Render.
  glutSwapBuffers();

  pthread_rwlock_unlock(&this->render_lock);
}

void VideoRenderer::HandleResize(const int width, const int height) {
  // Set up a 2D projection. This is mostly black magic.
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // 3D is for scrubs.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  // Alpha is good. Don't be beta.
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  this->window_width = width;
  this->window_height = height;
}
