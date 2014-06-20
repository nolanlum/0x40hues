#ifndef HUES_VIDEO_RENDERER_H_
#define HUES_VIDEO_RENDERER_H_

#include <common.h>

class VideoRenderer {

  DISALLOW_COPY_AND_ASSIGN(VideoRenderer)

  public:
    /**
     * Create a new VideoRenderer
     */
    VideoRenderer(); //TODO: I have no idea what needs to be initialized

    /**
     * Clean up the window before closing
     */
    ~VideoRenderer();

    /**
     * Makes an empty window
     */
    void InitWindow();

    /**
     * Draw a single frame.
     * VideoRenderer probably shouldn't handle timing
     */
    void DrawFrame(); //TODO: Needs argument signature

    /**
     * Special beats.
     * Image/color changes on any of them
     */
    enum BEAT {
      VERTICAL_BLUR,
      HORIZONTAL_BLUR,
      NO_BLUR,
      BLACKOUT,
      SHORT_BLACKOUT,
      COLOR_ONLY,
      IMAGE_ONLY
    };
  private:
}

#endif
