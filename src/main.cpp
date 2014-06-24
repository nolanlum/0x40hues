#include <iostream>

#include <respack.hpp>
#include <video_renderer.hpp>

int main(int argc, char **argv) {
  ResourcePack pack("../respacks/Default/");
  pack.Init();

  //VideoRenderer v;
  //v.InitWindow(argc, argv);

  return 0;
}
