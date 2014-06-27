#include <pthread.h>

#include <iostream>

#include <respack.hpp>
#include <video_renderer.hpp>

int main(int argc, char **argv) {
  // We need to call pthreads so we link it at runtime.
  // So here we have it (until we use pthreads for real).
  // See http://stackoverflow.com/questions/20007961/error-running-a-compiled-c-file-uses-opengl-error-inconsistency-detected
  pthread_getconcurrency();

  ResourcePack pack("../respacks/Default/");
  pack.Init();

  //VideoRenderer v;
  //v.InitWindow(argc, argv);

  return 0;
}
