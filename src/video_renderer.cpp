#include <iostream>

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

#include <common.h>
#include <video_renderer.h>

extern void DrawFrame();

VideoRenderer::VideoRenderer() {}
VideoRenderer::~VideoRenderer() {}

void VideoRenderer::InitWindow(int argc, char **argv) {
  glutInit(&argc, argv);
  glutCreateWindow("0x40 Hues");
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(50, 50);
  glutDisplayFunc(DrawFrame);
  glutMainLoop();
}

void DrawFrame() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
  glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)
  glFlush(); //Render
}
