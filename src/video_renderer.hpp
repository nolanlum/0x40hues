#ifndef HUES_VIDEO_RENDERER_H_
#define HUES_VIDEO_RENDERER_H_

#include <glew.h>
#include <pthread.h>

#include <unordered_map>

#include <common.hpp>
#include <respack.hpp>

// Forward declarations of HuesRenderer callbacks.
namespace HuesRenderer {
  void DrawFrameCallback();
  void ResizeCallback(const int width, const int height);
  void TimerCallback(int);
}

// VideoRenderer class.
class VideoRenderer {

  DISALLOW_COPY_AND_ASSIGN(VideoRenderer)

  public:
    /**
     * Create a new VideoRenderer.
     */
    VideoRenderer();

    /**
     * Clean up the window before closing.
     */
    ~VideoRenderer();

    /** Initializes GLUT and other OpenGL state. */
    void Init(int argc, char *argv[]);

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
    void LoadTextures(const ResourcePack &respack);

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
    // GLUT callback functions are declared friend so they can access private rendering functions.
    // Yeah it doesn't make much sense.
    friend void HuesRenderer::DrawFrameCallback();
    friend void HuesRenderer::ResizeCallback(const int, const int);

    /** Compiles our hard light (and eventually Gaussian blur) shaders. */
    void CompileShaders();
    /** Compiles a shader. Boilerplate sucks. */
    GLuint CompileShader(const char *&shader_text, GLenum shader_type);

    /** Draws a frame based on the parameters currently set in this class instance. */
    void DrawFrame();
    /** Updates class info based on a window resize event. */
    void HandleResize(const int width, const int height);

    unordered_map<string, GLuint> textures;
    unordered_map<string, ImageResource*> images;

    int window_height = -1;
    int window_width = -1;

    ImageResource *current_image = NULL;
    int current_color = 0;

    GLuint pass_through_vertex_shader;
    GLuint hard_light_fragment_shader;

    // Stores the shader program for blending the image with the color.
    struct {
      GLuint id;
      GLuint BaseImage;
      GLuint BlendColor;
    } image_blend_shaderprogram;

    pthread_rwlock_t render_lock;

    // THIS IS HELL AND YOU ARE THE DEVIL.
    static VideoRenderer *instance;

    static const char *kPassThroughVertexShader;
    static const char *kHardLightFragmentShader;
};

#endif
