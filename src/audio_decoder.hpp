#ifndef HUES_AUDIO_DECODER_H_
#define HUES_AUDIO_DECODER_H_

#include <utility>
#include <vector>

#include <mad.h>

#include <common.hpp>

using namespace std;

/**
 * Dithering routines borrowed from MADplay.
 */
class Dither {
  DISALLOW_COPY_AND_ASSIGN(Dither)

  public:

    Dither() {}
    ~Dither() {}

    signed long DitherUpdate(int bits, mad_fixed_t sample);

  private:

    inline unsigned long prng() { return (prng_state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL; }

    mad_fixed_t error[3];
    mad_fixed_t prng_state;
};

/**
 * This is a utility class for using the MAD MP# audio decoding library. It provides the decoding
 * logic, as well as PCM output functionality.
 *
 * I'm not documenting the rest of this class definition because I'm lazy.
 */
class AudioDecoder {
  DISALLOW_COPY_AND_ASSIGN(AudioDecoder)

  public:

    AudioDecoder(const uint8_t* const buffer, const int length) :
        audio_data(buffer), audio_data_length(length) {}
    ~AudioDecoder() {}

    uint8_t* Decode(int *sample_count, int *channel_count);

    static enum mad_flow MadInputCallback(void *decoder, struct mad_stream *stream);
    static enum mad_flow MadOutputCallback(void *decoder, struct mad_header const *header,
        struct mad_pcm *pcm);
    static enum mad_flow MadErrorCallback(void *decoder, struct mad_stream *stream,
        struct mad_frame *frame);

  private:

    Dither left_dither, right_dither;

    bool input_read;

    int sample_count = 0;
    int channel_count = 0;
    vector<pair<int, uint8_t*>> decoded_buffers;

    const uint8_t* const audio_data;
    const int audio_data_length;

};

#endif // HUES_AUDIO_DECODER_H_
