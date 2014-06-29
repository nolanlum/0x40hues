#include <assert.h>

#include <common.hpp>
#include <video_renderer.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

const char *VideoRenderer::kPassThroughVertexShader = R"END(
varying vec2 v_texCoord;

void main() {
  gl_Position = ftransform();
  v_texCoord = gl_Position.xy * vec2(0.5) + vec2(0.5);
}
)END";

const char *VideoRenderer::kHardLightFragmentShader = R"END(
uniform sampler2D BaseImage;
uniform vec4 BlendColor;

varying vec2 v_texCoord;

// Apply alpha channel to the texture by blending with white.
void applyAlpha(in vec4 base, out vec3 result) {
  result = mix(vec3(1.0), base.rgb, vec3(base.a));
}

// Alpha-unaware hard light blend function.
void hardLight(in vec3 base, in vec3 blend, out vec3 result) {
  vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);
  vec3 white = vec3(1.0);

  float luminance = dot(blend, lumCoeff);
  if (luminance < 0.5) {
    result = max(base + 2.0 * blend - 1.0, vec3(0.0));
  } else {
    result = min(base + 2.0 * (blend - 0.5), vec3(1.0));
  }
}

void main() {
  vec3 base;
  vec3 blend = vec3(BlendColor);
  vec3 result;

  // Apply hard light blend with .7 opacity.
  applyAlpha(texture2D(BaseImage, v_texCoord), base);
  hardLight(base, blend, result);
  result = mix(base, result, vec3(.7));
  gl_FragColor = vec4(result, 1);
}
)END";

const char *VideoRenderer::kGaussianXVertexShader = R"END(
// From [0, .5]. Provided example was .028
uniform float BlurRadius;

varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];

void main()
{
    gl_Position = ftransform();
    v_texCoord = gl_Position.xy * vec2(0.5) + vec2(0.5);

    float radiusStep = BlurRadius / 7;
    v_blurTexCoords[ 0] = vec2(max(0, v_texCoord.x + -7 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 1] = vec2(max(0, v_texCoord.x + -6 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 2] = vec2(max(0, v_texCoord.x + -5 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 3] = vec2(max(0, v_texCoord.x + -4 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 4] = vec2(max(0, v_texCoord.x + -3 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 5] = vec2(max(0, v_texCoord.x + -2 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 6] = vec2(max(0, v_texCoord.x + -1 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 7] = vec2(min(1, v_texCoord.x +  1 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 8] = vec2(min(1, v_texCoord.x +  2 * radiusStep), v_texCoord.y);
    v_blurTexCoords[ 9] = vec2(min(1, v_texCoord.x +  3 * radiusStep), v_texCoord.y);
    v_blurTexCoords[10] = vec2(min(1, v_texCoord.x +  4 * radiusStep), v_texCoord.y);
    v_blurTexCoords[11] = vec2(min(1, v_texCoord.x +  5 * radiusStep), v_texCoord.y);
    v_blurTexCoords[12] = vec2(min(1, v_texCoord.x +  6 * radiusStep), v_texCoord.y);
    v_blurTexCoords[13] = vec2(min(1, v_texCoord.x +  7 * radiusStep), v_texCoord.y);
}
)END";

const char *VideoRenderer::kGaussianYVertexShader = R"END(
uniform float BlurRadius;

varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];

void main()
{
    gl_Position = ftransform();
    v_texCoord = gl_Position.xy * vec2(0.5) + vec2(0.5);

    float radiusStep = BlurRadius / 7;
    v_blurTexCoords[ 0] = vec2(v_texCoord.x, max(0, v_texCoord.y + -7 * radiusStep));
    v_blurTexCoords[ 1] = vec2(v_texCoord.x, max(0, v_texCoord.y + -6 * radiusStep));
    v_blurTexCoords[ 2] = vec2(v_texCoord.x, max(0, v_texCoord.y + -5 * radiusStep));
    v_blurTexCoords[ 3] = vec2(v_texCoord.x, max(0, v_texCoord.y + -4 * radiusStep));
    v_blurTexCoords[ 4] = vec2(v_texCoord.x, max(0, v_texCoord.y + -3 * radiusStep));
    v_blurTexCoords[ 5] = vec2(v_texCoord.x, max(0, v_texCoord.y + -2 * radiusStep));
    v_blurTexCoords[ 6] = vec2(v_texCoord.x, max(0, v_texCoord.y + -1 * radiusStep));
    v_blurTexCoords[ 7] = vec2(v_texCoord.x, min(1, v_texCoord.y +  1 * radiusStep));
    v_blurTexCoords[ 8] = vec2(v_texCoord.x, min(1, v_texCoord.y +  2 * radiusStep));
    v_blurTexCoords[ 9] = vec2(v_texCoord.x, min(1, v_texCoord.y +  3 * radiusStep));
    v_blurTexCoords[10] = vec2(v_texCoord.x, min(1, v_texCoord.y +  4 * radiusStep));
    v_blurTexCoords[11] = vec2(v_texCoord.x, min(1, v_texCoord.y +  5 * radiusStep));
    v_blurTexCoords[12] = vec2(v_texCoord.x, min(1, v_texCoord.y +  6 * radiusStep));
    v_blurTexCoords[13] = vec2(v_texCoord.x, min(1, v_texCoord.y +  7 * radiusStep));
}
)END";

const char *VideoRenderer::kGaussianFragmentShader = R"END(
precision mediump float;

uniform sampler2D Image;

varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];

void main()
{
  gl_FragColor = vec4(0.0);
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 0]) * 0.0044299121055113265;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 1]) * 0.00895781211794;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 2]) * 0.0215963866053;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 3]) * 0.0443683338718;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 4]) * 0.0776744219933;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 5]) * 0.115876621105;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 6]) * 0.147308056121;
  gl_FragColor += texture2D(Image, v_texCoord         ) * 0.159576912161;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 7]) * 0.147308056121;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 8]) * 0.115876621105;
  gl_FragColor += texture2D(Image, v_blurTexCoords[ 9]) * 0.0776744219933;
  gl_FragColor += texture2D(Image, v_blurTexCoords[10]) * 0.0443683338718;
  gl_FragColor += texture2D(Image, v_blurTexCoords[11]) * 0.0215963866053;
  gl_FragColor += texture2D(Image, v_blurTexCoords[12]) * 0.00895781211794;
  gl_FragColor += texture2D(Image, v_blurTexCoords[13]) * 0.0044299121055113265;
}

)END";

// TODO: tune this parameter.
const float VideoRenderer::kFullStrengthBlurRadius = 0.1;

/** blur_x and blur_y are divided by this factor every decay tick. */
const float VideoRenderer::kBlurDecayFactorPerTick = 1.3f;

/** 60 ticks per second. */
const clock_t VideoRenderer::kBlurDecayTick = CLOCKS_PER_SEC / 60;

VideoRenderer *VideoRenderer::instance = NULL;

void VideoRenderer::DrawFrameCallback() {
  instance->DrawFrame();
}

void VideoRenderer::ResizeCallback(const int width, const int height) {
  instance->HandleResize(width, height);
}

void VideoRenderer::TimerCallback(int ignored) {
  instance->HandleTimerTick();
  glutPostRedisplay();

  // Cap at 500fps, I guess.
  glutTimerFunc(2, TimerCallback, 0);
}

VideoRenderer::VideoRenderer() {
  pthread_rwlock_init(&this->render_lock, NULL);
  pthread_mutex_init(&this->load_mutex, NULL);
  pthread_cond_init(&this->load_cv, NULL);
}

VideoRenderer::~VideoRenderer() {
  pthread_rwlock_destroy(&this->render_lock);
  pthread_mutex_destroy(&this->load_mutex);
  pthread_cond_destroy(&this->load_cv);

  for (auto const& kv : this->textures) {
    glDeleteTextures(1, &kv.second);
  }

  if (this->blur_fb.tex_id != 0) {
    glDeleteTextures(1, &this->blur_fb.tex_id);
  }
  glDeleteFramebuffers(1, &this->blur_fb.id);

  glDeleteShader(this->pass_through_vertex_shader);
  glDeleteShader(this->hard_light_fragment_shader);
  glDeleteProgram(this->image_blend_shaderprogram.id);

  glDeleteShader(this->gaussian_x_vertex_shader);
  glDeleteShader(this->gaussian_y_vertex_shader);
  glDeleteShader(this->gaussian_fragment_shader);
  glDeleteProgram(this->blur_x_shaderprogram.id);
  glDeleteProgram(this->blur_y_shaderprogram.id);
}

void VideoRenderer::Init(int argc, char *argv[]) {
  // Set preferred window size and location.
  glutInitWindowSize(1280, 720);
  glutInitWindowPosition(50, 50);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
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

  // Create framebuffer object.
  glGenFramebuffers(1, &this->blur_fb.id);

  // Set callback functions.
  glutDisplayFunc(DrawFrameCallback);
  glutReshapeFunc(ResizeCallback);
  glutTimerFunc(0, TimerCallback, 0);

  // Clear the window and display a solid color.
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();
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
  // Compile the shaders, then link the blending program.
  this->pass_through_vertex_shader =
      this->CompileShader(VideoRenderer::kPassThroughVertexShader, GL_VERTEX_SHADER);
  this->hard_light_fragment_shader =
      this->CompileShader(VideoRenderer::kHardLightFragmentShader, GL_FRAGMENT_SHADER);

  this->image_blend_shaderprogram.id = glCreateProgram();
  glAttachShader(this->image_blend_shaderprogram.id, this->pass_through_vertex_shader);
  glAttachShader(this->image_blend_shaderprogram.id, this->hard_light_fragment_shader);
  glLinkProgram(this->image_blend_shaderprogram.id);

  this->image_blend_shaderprogram.BaseImage =
      glGetUniformLocation(this->image_blend_shaderprogram.id, "BaseImage");
  this->image_blend_shaderprogram.BlendColor =
      glGetUniformLocation(this->image_blend_shaderprogram.id, "BlendColor");

  // Compile the shaders for gaussian blur, then link two separate programs.
  this->gaussian_x_vertex_shader =
      this->CompileShader(VideoRenderer::kGaussianXVertexShader, GL_VERTEX_SHADER);
  this->gaussian_y_vertex_shader =
      this->CompileShader(VideoRenderer::kGaussianYVertexShader, GL_VERTEX_SHADER);
  this->gaussian_fragment_shader =
      this->CompileShader(VideoRenderer::kGaussianFragmentShader, GL_FRAGMENT_SHADER);

  this->blur_x_shaderprogram.id = glCreateProgram();
  glAttachShader(this->blur_x_shaderprogram.id, this->gaussian_x_vertex_shader);
  glAttachShader(this->blur_x_shaderprogram.id, this->gaussian_fragment_shader);
  glLinkProgram(this->blur_x_shaderprogram.id);
  this->blur_x_shaderprogram.BlurRadius =
      glGetUniformLocation(this->blur_x_shaderprogram.id, "BlurRadius");
  this->blur_x_shaderprogram.Image =
      glGetUniformLocation(this->blur_x_shaderprogram.id, "Image");

  this->blur_y_shaderprogram.id = glCreateProgram();
  glAttachShader(this->blur_y_shaderprogram.id, this->gaussian_y_vertex_shader);
  glAttachShader(this->blur_y_shaderprogram.id, this->gaussian_fragment_shader);
  glLinkProgram(this->blur_y_shaderprogram.id);
  this->blur_y_shaderprogram.BlurRadius =
      glGetUniformLocation(this->blur_y_shaderprogram.id, "BlurRadius");
  this->blur_y_shaderprogram.Image =
      glGetUniformLocation(this->blur_y_shaderprogram.id, "Image");
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
      GLchar* log = new GLchar[log_len];
      glGetInfoLogARB(shader, log_len, NULL, log);
      ERR("Error compiling shader! Output on next lines.");
      cout << log;
      delete[] log;

      // Might as well fail because shaders are important for this application.
      exit(EXIT_FAILURE);
    }
  }

  return shader;
}

void VideoRenderer::LoadTextures(const ResourcePack &respack) {
  pthread_mutex_lock(&this->load_mutex);
  this->textures_loaded = false;
  pthread_mutex_unlock(&this->load_mutex);

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
        delete[] image_bytes;
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
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, (GLuint) NULL);

    delete[] image_bytes;

    texture_name_list.push_back(image->GetName());
    this->images[image->GetName()] = image;
    this->textures[image->GetName()] = texture;
  }

  pthread_mutex_lock(&this->load_mutex);
  this->textures_loaded = true;
  pthread_cond_broadcast(&this->load_cv);
  pthread_mutex_unlock(&this->load_mutex);
}

void VideoRenderer::WaitForTextureLoad() {
  pthread_mutex_lock(&this->load_mutex);
  if (this->textures_loaded) {
    pthread_mutex_unlock(&this->load_mutex);
    return;
  }

  pthread_cond_wait(&this->load_cv, &this->load_mutex);
  pthread_mutex_unlock(&this->load_mutex);
}

bool VideoRenderer::SetImage(const string& image_name, const AudioResource::Beat transition) {
  pthread_rwlock_wrlock(&this->render_lock);

  if (this->images.find(image_name) == this->images.end()) {
    ERR("Render image [" + image_name + "] failed: not loaded!");
    pthread_rwlock_unlock(&this->render_lock);
    return false;
  }

  this->current_image = this->images[image_name];

  string transition_type = "";
  switch(transition) {
    case AudioResource::Beat::VERTICAL_BLUR:
      this->blur_y = 1.f;
      transition_type = ", type: [Y_BLUR]";
      break;
    case AudioResource::Beat::HORIZONTAL_BLUR:
      this->blur_x = 1.f;
      transition_type = ", type: [X_BLUR]";
      break;
    case AudioResource::Beat::NO_BLUR:
      transition_type = ", type: [PLAIN]";
      break;
    case AudioResource::Beat::IMAGE_ONLY:
      transition_type = ", type: [IMG_ONLY]";
      break;
    default: break;
  }

  glutPostRedisplay();
  pthread_rwlock_unlock(&this->render_lock);

  LOG("Transition in: [" + image_name + "]" + transition_type);
  return true;
}

bool VideoRenderer::SetColor(const int color_index) {
  this->current_color = (color_index < 0) ? 0 : (color_index % 0x40);
  return true;
}

void VideoRenderer::SetImage(const AudioResource::Beat transition) {
  this->SetImage(this->texture_name_list[rand() %this->texture_name_list.size()], transition);
}
void VideoRenderer::SetColor() {
  this->SetColor(rand());
}

void VideoRenderer::DrawFrame() {
  pthread_rwlock_rdlock(&this->render_lock);

  // Calculate the color based on the index, normalized to [0,1].
  float red = (this->current_color & 0b11) / 3.f;
  float green = ((this->current_color & 0b1100) >> 2) / 3.f;
  float blue = ((this->current_color & 0b110000) >> 4) / 3.f;
  glClear(GL_COLOR_BUFFER_BIT);

  // Make sure we have a image to draw, first.
  if (this->current_image) {
    // Render to texture if we want to blur.
    if (this->blur_x > 0 || this->blur_y > 0) {
      this->MarkRenderToTexture();
    }

    // Set image blend program.
    glUseProgram(this->image_blend_shaderprogram.id);

    // Set shader program uniforms.
    GLuint texture = this->textures[this->current_image->GetName()];
    glUniform1i(this->image_blend_shaderprogram.BaseImage, 0);
    glUniform4f(this->image_blend_shaderprogram.BlendColor, red, green, blue, 1);

    // Bind the texture and draw a rectangle.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(this->window_width, 0);
        glVertex2f(this->window_width, this->window_height);
        glVertex2f(0, this->window_height);
    glEnd();

    // Now handle drawing the blurred image to screen.
    if (this->blur_x > 0 || this->blur_y > 0) {
      // Not ideal, but there needs to be a way to pick the blur direction active when they're both
      // above zero? Maybe?
      struct blurprogram *prog =
          (this->blur_x > this->blur_y) ? &this->blur_x_shaderprogram : &this->blur_y_shaderprogram;
      float blur = (this->blur_x > this->blur_y) ? this->blur_x : this->blur_y;

      this->MarkRenderToScreen();
      glUseProgram(prog->id);
      glUniform1i(prog->Image, 0);
      glUniform1f(prog->BlurRadius, VideoRenderer::kFullStrengthBlurRadius * blur);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, this->blur_fb.tex_id);
      glBegin(GL_QUADS);
          glVertex2f(0, 0);
          glVertex2f(this->window_width, 0);
          glVertex2f(this->window_width, this->window_height);
          glVertex2f(0, this->window_height);
      glEnd();
    }
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

  this->window_width = width;
  this->window_height = height;

  this->InitFramebuffer();
}

void VideoRenderer::InitFramebuffer() {
  // We need to bind the framebuffer before doing anything related to it, it seems.
  glBindFramebuffer(GL_FRAMEBUFFER, this->blur_fb.id);

  // Delete the previous texture if there was one.
  if (this->blur_fb.tex_id != 0) {
    glDeleteTextures(1, &this->blur_fb.tex_id);
    this->blur_fb.tex_id = 0;
  }

  // Generate a texture and set parameters.
  glGenTextures(1, &this->blur_fb.tex_id);
  glBindTexture(GL_TEXTURE_2D, this->blur_fb.tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->window_width, this->window_height, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      this->blur_fb.tex_id, 0);

  // Tell the framebuffer to draw to the attached color buffer.
  static GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, DrawBuffers);

  // Unbind the current texture and framebuffer.
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VideoRenderer::HandleTimerTick() {
  if ((this->blur_x > 0 || this->blur_y > 0)
        && (clock() - this->last_blur_decay_tick >= VideoRenderer::kBlurDecayTick)) {
    this->blur_x /= VideoRenderer::kBlurDecayFactorPerTick;
    this->blur_y /= VideoRenderer::kBlurDecayFactorPerTick;

    this->last_blur_decay_tick = clock();
  }

  this->blur_x = this->blur_x < .001 ? 0 : this->blur_x;
  this->blur_y = this->blur_y < .001 ? 0 : this->blur_y;
}

void VideoRenderer::MarkRenderToTexture() {
  glBindFramebuffer(GL_FRAMEBUFFER, this->blur_fb.id);
  glClear(GL_COLOR_BUFFER_BIT);
}
void VideoRenderer::MarkRenderToScreen() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
}
