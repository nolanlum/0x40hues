#ifndef HUES_VIDEO_RENDERER_H_
#define HUES_VIDEO_RENDERER_H_

#include <common.hpp>
#include <respack.hpp>

namespace HuesRenderer {
  void DrawFrameCallback();
}

class VideoRenderer {

  DISALLOW_COPY_AND_ASSIGN(VideoRenderer)

  public:
    /**
     * Create a new VideoRenderer
     */
    VideoRenderer(int argc, char *argv[]); //TODO: I have no idea what needs to be initialized

    /**
     * Clean up the window before closing
     */
    ~VideoRenderer();

    /**
     * Creates the program window, registers GLUT callbacks, then runs the GLUT event loop
     * on the current thread.
     */
    void DoGlutLoop();

    /**
     * Loads into memory all images contained in the given resource pack.
     *
     * @param respack the resource pack to load image textures from.
     */
    void LoadTextures(ResourcePack *respack);

    /**
     * Set an Image for the next DrawFrame()
     *
     * @param image_name the name of the next image to show.
     * @param transition the type of beat transition to draw.
     * @return <code>true</code> if the image exists, and was successfully marked for redraw.
     *         <code>false</code> otherwise.
     */
    bool SetImage(const string& image_name, const AudioResource::Beat transition);

    /**
     * Set a color for the next DrawFrame()
     *
     * @param color_index the index of the color to draw. Must be in the interval [0,0x40).
     *                    (Get it?)
     * @return <code>true</code> if the color was updated successfully, <code>false</code>
     *         otherwise.
     */
    bool SetColor(const int color_index);

  private:
    /**
     * GLUT callback function for drawing a frame.
     *
     * This function uses the state of the statically available VideoRenderer to decide what to
     * draw, and draws it.
     */
    friend void HuesRenderer::DrawFrameCallback();

};

#endif
