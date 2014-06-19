#ifdef _HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>

#include <pugixml.hpp>

#include <respack.h>

int main() {
  ResourcePack pack("../../respacks/Default/");
  LOG("Hello world.");
  DEBUG("Debug, world!");

  return 0;
}
