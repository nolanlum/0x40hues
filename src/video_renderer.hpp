#ifndef HUES_VIDEO_RENDERER_H_
#define HUES_VIDEO_RENDERER_H_

#include <glew.h>
#include <pthread.h>

#include <cmath>
#include <unordered_map>

#include <common.hpp>
#include <respack.hpp>

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
     * Blocks until all textures are loaded into memory.
     */
    void WaitForTextureLoad();

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
     * Set an Image for the next DrawFrame(). The Image is randomly chosen among all loaded images.
     *
     * @param transition the type of beat transition to draw.
     */
    void SetImage(const AudioResource::Beat transition);

    /**
     * Set a color for the next DrawFrame()
     *
     * @param color_index the index of the color to draw. Must be in the interval [0,0x40).
     *                    (Get it?)
     * @return <code>true</code> if the color was updated successfully, <code>false</code>
     *         otherwise.
     */
    bool SetColor(const int color_index);

     /**
     * Set a color for the next DrawFrame(). The color is randomly chosen.
     */
    void SetColor();

  private:

    static void DrawFrameCallback();
    static void ResizeCallback(const int, const int);
    static void TimerCallback(int);

    /** Compiles our hard light (and eventually Gaussian blur) shaders. */
    void CompileShaders();
    /** Compiles a shader. Boilerplate sucks. */
    GLuint CompileShader(const char *&shader_text, GLenum shader_type);

    /** Draws a frame based on the parameters currently set in this class instance. */
    void DrawFrame();
    /** Updates class info based on a window resize event. */
    void HandleResize(const int width, const int height);
    /** Handle a GLUT timer event */
    void HandleTimerTick();

    /** (Re-)Initializes a FBO to the current screen dimensions. */
    void InitFramebuffer();

    void MarkRenderToTexture();
    void MarkRenderToScreen();

    vector<string> texture_name_list;
    unordered_map<string, GLuint> textures;
    unordered_map<string, ImageResource*> images;

    bool textures_loaded = false;

    int window_height = -1;
    int window_width = -1;

    ImageResource *current_image = NULL;
    int current_color = 0;

    // A number, in [0,1], that determines what portion of the full strength blur radius we use.
    // Clamped to the nearest thousandth.
    float blur_x = 0.f;
    float blur_y = 0.f;
    clock_t last_blur_decay_tick;

    GLuint pass_through_vertex_shader;
    GLuint hard_light_fragment_shader;

    GLuint gaussian_x_vertex_shader;
    GLuint gaussian_y_vertex_shader;
    GLuint gaussian_fragment_shader;

    // Stores the shader program for blending the image with the color.
    struct {
      GLuint id;
      GLuint BaseImage;
      GLuint BlendColor;
    } image_blend_shaderprogram;

    // Stores the shader program for blurring the image in the x direction.
    struct blurprogram {
      GLuint id;
      GLuint BlurRadius;
      GLuint Image;
    } blur_x_shaderprogram;

    // Stores the shader program for blurring the image in the y direction.
    struct blurprogram blur_y_shaderprogram;

    // Stores texture and framebuffer info when rendering to texture for Gaussian blur.
    struct {
      GLuint id;
      GLuint tex_id;
    } blur_fb;

    pthread_rwlock_t render_lock;
    pthread_mutex_t load_mutex;
    pthread_cond_t load_cv;

    // THIS IS HELL AND YOU ARE THE DEVIL.
    static VideoRenderer *instance;

    static const float kFullStrengthBlurRadius;
    static const float kBlurDecayFactorPerTick;
    static const clock_t kBlurDecayTick;

    static const char *kPassThroughVertexShader;
    static const char *kHardLightFragmentShader;
    static const char *kGaussianXVertexShader;
    static const char *kGaussianYVertexShader;
    static const char *kGaussianFragmentShader;
};

#endif
