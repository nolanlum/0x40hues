#include <iostream>

#include <pugixml.hpp>

#include <respack.h>

#ifdef _HAVE_WINDOWS_H
#include <windows.h>
#endif

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

int main() {
  ResourcePack pack("../../respacks/Default/");
  LOG("Hello world.");
  DEBUG("Debug, world!");

  return 0;
}
