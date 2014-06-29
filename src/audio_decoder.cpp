#include <assert.h>
#include <sys/stat.h>

#include <cstdio>

#include <audio_decoder.hpp>

// This number is magic and I don't know where it came from.
#define MAD_DELAY 529
#define MP3_FRAME_SIZE 1152

// Via https://code.google.com/p/squeezelite/source/browse/mad.c
// Reformatted to conform.
void AudioDecoder::CheckLameGaplessHeader() {
  const uint8_t *ptr = this->audio_data;

  // First check to see if this is really a MP3 file (the internet told me to!)
  if (*ptr == 0xFF && (*(ptr + 1) & 0xF0) == 0xF0 && audio_data_length > 180) {
    int frame_count = 0, enc_delay = 0, enc_padding = 0;
    int flags;

    // Hardcoded locations for a 2-channel MP3.
    if (!memcmp(ptr + 36, "Xing", 4) || !memcmp(ptr + 36, "Info", 4)) {
      ptr += 36 + 7;
    // Hardcoded locations for a mono MP3.
    } else if (!memcmp(ptr + 21, "Xing", 4) || !memcmp(ptr + 21, "Info", 4)) {
      ptr += 21 + 7;
    }

    flags = *ptr;

    if (flags & 0x01) {
      frame_count = *(ptr + 1) << 24
          | *(ptr + 2) << 16
          | *(ptr + 3) << 8
          | *(ptr + 4);
      ptr += 4;
    }
    if (flags & 0x02) {
      ptr += 4;
    }
    if (flags & 0x04) {
      ptr += 100;
    }
    if (flags & 0x08) {
      ptr += 4;
    }

    if (!!memcmp(ptr + 1, "LAME", 4)) {
      return;
    }

    ptr += 22;


    enc_delay   = (*ptr << 4 | *(ptr + 1) >> 4) + MAD_DELAY;
    enc_padding = (*(ptr + 1) & 0xF) << 8 | (*(ptr + 2) & 0xFFF);

    this->gapless.total_samples = frame_count * 1152 - enc_delay - enc_padding;
    this->gapless.delay         = enc_delay;
    this->gapless.padding       = enc_padding;
  }
}


uint8_t* AudioDecoder::Decode(int *sample_count, int *channel_count, int *sample_rate) {
  struct mad_decoder decoder;
  input_read = false;

  this->CheckLameGaplessHeader();

  mad_decoder_init(&decoder, this, AudioDecoder::MadInputCallback, /* header */ NULL,
      /* filter */ NULL, AudioDecoder::MadOutputCallback, AudioDecoder::MadErrorCallback,
      /* message */ NULL);
  mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
  mad_decoder_finish(&decoder);

  // 16 bits per sample. Also ensure it's word-aligned.
  int decoded_buffer_size = (this->channel_count * 2 * this->sample_count);
  int padding = ((decoded_buffer_size + 3) & ~3) - decoded_buffer_size;
  decoded_buffer_size += padding;

  uint8_t *buffer = new uint8_t[decoded_buffer_size];
  if (!buffer) {
    ERR("Couldn't allocate [" + to_string(decoded_buffer_size)
        + "] bytes of memory to return decoded audio data!");
    return NULL;
  }

  uint8_t *cur_pos = buffer;
  for(pair<int, uint8_t*> buf : this->decoded_buffers) {
    memcpy(cur_pos, buf.second, buf.first);
    cur_pos += buf.first;

    delete[] buf.second;
  }
  memset(cur_pos, 0, padding);
  this->decoded_buffers.clear();

  if (this->sample_count != this->gapless.total_samples) {
    assert(this->sample_count - this->gapless.delay - this->gapless.padding
        == this->gapless.total_samples);

    this->sample_count = this->gapless.total_samples;
    buffer += this->gapless.delay * 2 * this->channel_count;

    char gapless_info[100];
    snprintf(gapless_info, 100, "Gapless delay: %u samples; padding: %u samples.",
        this->gapless.delay, this->gapless.padding);
    LOG(gapless_info);
  }

  if (sample_count) {
    *sample_count = this->sample_count;
  }
  if (channel_count) {
    *channel_count = this->channel_count;
  }
  if (sample_rate) {
    *sample_rate = this->sample_rate;
  }

  LOG("Decoded [" + to_string(this->sample_count) + "] samples after gapless playback correction.");

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
  _this->sample_rate          = pcm->samplerate;
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
  snprintf(errorbuffer, 1024, "libmad decoding error 0x%04x (%s) at byte offset %llu.",
    stream->error, mad_stream_errorstr(stream), stream->this_frame - _this->audio_data);
  ERR(errorbuffer);

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
