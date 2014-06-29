#ifndef HUES_AUDIO_RENDERER_H_
#define HUES_AUDIO_RENDERER_H_

#include <common.hpp>

struct AudioRendererPrivate;

class AudioRenderer {
  DISALLOW_COPY_AND_ASSIGN(AudioRenderer)

  public:
    AudioRenderer();
    ~AudioRenderer();

    /**
     * Initializes the platform's audio backend to play 16-bit little-endian PCM audio.
     *
     * @param channels the number of audio channels to expect (interleaved PCM).
     * @param sample_rate the sample rate of the audio.
     * @return <code>true</code> if initialization was successful, <code>false</code> otherwise.
     */
    bool Init(const int channels, const int sample_rate);

    void PlayAudio(const uint8_t* const pcm_data, const size_t len);

  private:
    /** Each platform can define their own version of the AudioRendererPrivate struct. */
    struct AudioRendererPrivate *_;
};

#endif // HUES_AUDIO_RENDERER_H_
