#include <pthread.h>
#include <unistd.h>

#include <iostream>

#include <respack.hpp>
#include <video_renderer.hpp>

static VideoRenderer *v;
static ResourcePack *p;

// Video renderer thread entry point.
void* video_renderer(void *) {
  const char *argv[] { "" };
  v->Init(1, const_cast<char **>(argv));
  v->LoadTextures(*p);
  v->DoGlutLoop();
  pthread_exit(EXIT_SUCCESS);
  return 0;
}

int main(int argc, char **argv) {
  p = new ResourcePack("../respacks/Default/");
  p->Init();
  v = new VideoRenderer();

  pthread_t render_thread_id;
  pthread_create(&render_thread_id, NULL, video_renderer, NULL);

  // What is a race condition?
  vector<ImageResource*> imgs;
  p->GetAllImages(imgs);
  for (;;) {
    usleep(((rand() % 1000) + 500) * 1000);
    v->SetImage(imgs[rand() % imgs.size()]->GetName(), AudioResource::Beat::NO_BLUR);
    v->SetColor(rand() % 0x40);
  }

  pthread_join(render_thread_id, NULL);
  pthread_exit(0);
}

