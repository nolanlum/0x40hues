#include <common.hpp>
#include <video_renderer.hpp>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
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

  void IdleCallback() {
    glutPostRedisplay();
  }

}

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

  glutCreateWindow("0x40 Hues");

  // Set callback functions.
  glutDisplayFunc(HuesRenderer::DrawFrameCallback);
  glutReshapeFunc(HuesRenderer::ResizeCallback);
  glutIdleFunc(HuesRenderer::IdleCallback);

  // Clear the window and display a solid color.
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();

  glClearColor(.3, .6, .7, 1);
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

void VideoRenderer::LoadTextures(const ResourcePack &respack) {
  vector<ImageResource*> images;
  respack.GetAllImages(images);

  for(ImageResource *image : images) {
    int width;
    int height;
    int color_type;

    LOG("Loading [" + image->GetName() + "] into texture memory.");
    png_byte *image_bytes = image->ReadAndDecode(&width, &height, &color_type);

    GLint texture_format;
    switch(color_type) {
      case PNG_COLOR_TYPE_RGB_ALPHA:
        texture_format = GL_RGBA;
        break;
      default:
        ERROR("Unsupported libpng color type: [" + to_string(color_type) + "].");
        free(image_bytes);
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, /* level of detail number */0, GL_LUMINANCE_ALPHA,
        width, height, /* border */ 0, texture_format, GL_UNSIGNED_BYTE, image_bytes);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, (GLuint) NULL);

    free(image_bytes);
    this->images[image->GetName()] = image;
    this->textures[image->GetName()] = texture;
  }
}

bool VideoRenderer::SetImage(const string& image_name, const AudioResource::Beat transition) {
  pthread_rwlock_wrlock(&this->render_lock);

  if (this->images.find(image_name) == this->images.end()) {
    ERROR("Render image [" + image_name + "] failed: not loaded!");
    pthread_rwlock_unlock(&this->render_lock);
    return false;
  }

  this->current_image = this->images[image_name];
  glutPostRedisplay();

  pthread_rwlock_unlock(&this->render_lock);

  LOG("Starting transition to [" + image_name + "].");
  return true;
}

void VideoRenderer::DrawFrame() {
  pthread_rwlock_rdlock(&this->render_lock);

  glClear(GL_COLOR_BUFFER_BIT);

  // Make sure we have a image to draw, first.
  if (this->current_image) {
    // Bind the texture and draw a rectangle.
    glBindTexture(GL_TEXTURE_2D, this->textures[this->current_image->GetName()]);
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
