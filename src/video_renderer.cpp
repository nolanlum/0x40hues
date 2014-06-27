#include <iostream>

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

// All GLUT callbacks are contained in the HuesRenderer namespace to avoid polluting the global
// namespace with "private" functions. It's gross.
namespace HuesRenderer {

  // This is terrible and not at all thread-safe but it can't be helped.
  static VideoRenderer *huesVideoRenderer;

  void DrawFrameCallback() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)
    glFlush(); //Render
  }

}

VideoRenderer::VideoRenderer(int argc, char *argv[]) {
  glutInit(&argc, argv);
}
VideoRenderer::~VideoRenderer() {}

void VideoRenderer::DoGlutLoop() {
  HuesRenderer::huesVideoRenderer = this;

  glutInitWindowSize(800, 600);
  glutInitWindowPosition(50, 50);

  glutCreateWindow("0x40 Hues");
  glutDisplayFunc(HuesRenderer::DrawFrameCallback);
  glutMainLoop();
}
