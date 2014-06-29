#include <sys/stat.h>

#include <audio_decoder.hpp>

uint8_t* AudioDecoder::Decode(int *sample_count, int *channel_count) {
  struct mad_decoder decoder;
  input_read = false;

  mad_decoder_init(&decoder, this, AudioDecoder::MadInputCallback, /* header */ NULL,
      /* filter */ NULL, AudioDecoder::MadOutputCallback, AudioDecoder::MadErrorCallback,
      /* message */ NULL);
  mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
  mad_decoder_finish(&decoder);

  // 16 bits per sample.
  int decoded_buffer_size = this->channel_count * 2 * this->sample_count;
  if (sample_count) {
    *sample_count = this->sample_count;
  }
  if (channel_count) {
    *channel_count = this->channel_count;
  }

  uint8_t *buffer = new uint8_t[decoded_buffer_size];
  if (!buffer) {
    ERR("Couldn't allocate [" + to_string(decoded_buffer_size)
        + "] bytes of memory to return decoded audio data!");
    return NULL;
  }

  uint8_t *cur_pos = buffer;
  for(pair<int, uint8_t*> buf : this->decoded_buffers) {
    memcpy(cur_pos, buf.second, buf.first);
    delete[] buf.second;
  }
  this->decoded_buffers.clear();

  return buffer;
}

enum mad_flow AudioDecoder::MadInputCallback(void *decoder, struct mad_stream *stream) {
  AudioDecoder *_this = static_cast<AudioDecoder*>(decoder);

  if (_this->input_read) {
    return MAD_FLOW_STOP;
  }

  // Send all the audio data to MAD in one go.
  mad_stream_buffer(stream, _this->audio_data, _this->audio_data_length);

  _this->input_read = true;

  return MAD_FLOW_CONTINUE;
}

enum mad_flow AudioDecoder::MadOutputCallback(void *decoder, struct mad_header const *header,
    struct mad_pcm *pcm) {
  AudioDecoder *_this = static_cast<AudioDecoder*>(decoder);

  _this->channel_count        = pcm->channels;
  int current_sample_count    = pcm->length;
  mad_fixed_t const *left_ch  = pcm->samples[0];
  mad_fixed_t const *right_ch = pcm->samples[1];

  // 16 bits per sample.
  int decoded_buffer_size = _this->channel_count * 2 * current_sample_count;
  uint8_t *buffer = new uint8_t[decoded_buffer_size];
  if (!buffer) {
    ERR("Unable to allocate [" + to_string(decoded_buffer_size) + "] bytes of memory!");
    return MAD_FLOW_BREAK;
  }
  _this->decoded_buffers.push_back(make_pair(decoded_buffer_size, buffer));
  _this->sample_count += current_sample_count;

  while (current_sample_count--) {
    // Output sample(s) in 16-bit signed little-endian PCM.
    signed int sample;

    sample = _this->left_dither.DitherUpdate(16, *left_ch++);
    *buffer++ = (sample >> 0) & 0xFF;
    *buffer++ = (sample >> 8) & 0xFF;

    if (_this->channel_count == 2) {
      sample = _this->right_dither.DitherUpdate(16, *right_ch++);
      *buffer++ = (sample >> 0) & 0xFF;
      *buffer++ = (sample >> 8) & 0xFF;
    }
  }

  return MAD_FLOW_CONTINUE;
}

enum mad_flow AudioDecoder::MadErrorCallback(void *decoder, struct mad_stream *stream,
    struct mad_frame *frame) {
  AudioDecoder *_this = static_cast<AudioDecoder*>(decoder);

  char errorbuffer[1024];
  sprintf(errorbuffer, "libmad decoding error 0x%04x (%s) at byte offset %llu.",
    stream->error, mad_stream_errorstr(stream), stream->this_frame - _this->audio_data);
  ERR(errorbuffer);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

signed long Dither::DitherUpdate(int bits, mad_fixed_t sample) {
  unsigned int scalebits;
  mad_fixed_t output, mask, random;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += this->error[0] - this->error[1] + this->error[2];

  this->error[2] = this->error[1];
  this->error[1] = this->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random = prng();
  output += (random & mask) - (prng_state & mask);

  prng_state = random;

  /* clip */
  if (output > MAX) {
    output = MAX;

    if (sample > MAX) {
      sample = MAX;
    }
  } else if (output < MIN) {
    output = MIN;

    if (sample < MIN) {
      sample = MIN;
    }
  }

  /* quantize */
  output &= ~mask;

  /* error feedback */
  this->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}
