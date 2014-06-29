#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <cstdio>
#include <cstring>
#include <string>

#include <audio_renderer.hpp>

using namespace std;

struct AudioRendererPrivate {
  HWAVEOUT hout;
  WAVEHDR wavebuf;
};

static void MMError(const string& function, const MMRESULT code) {
  TCHAR errbuf[MAXERRORLENGTH];
  waveOutGetErrorText(code, errbuf, MAXERRORLENGTH);
  ERR(function + ": " + errbuf);
}

bool AudioRenderer::Init(const int channels, const int sample_rate) {
  WAVEFORMATEX waveformat;
  MMRESULT result;
  UINT devId = WAVE_MAPPER;  // WAVE_MAPPER == choose system's default

  // Initialize WaveFormatEx struct.
  waveformat.wFormatTag = WAVE_FORMAT_PCM;
  waveformat.wBitsPerSample = 16;
  waveformat.nChannels = channels;
  waveformat.nSamplesPerSec = sample_rate;
  waveformat.nBlockAlign = waveformat.nChannels * waveformat.wBitsPerSample / 8;
  waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec * waveformat.nBlockAlign;

  result = waveOutOpen(&this->_->hout, devId, &waveformat, 0, 0, CALLBACK_NULL);

  if (result != MMSYSERR_NOERROR) {
    MMError("waveOutOpen()", result);
    return false;
  }

  return true;
}

void AudioRenderer::PlayAudio(const uint8_t* const pcm_data, const size_t len) {
  this->_->wavebuf.dwBufferLength = len;
  this->_->wavebuf.dwFlags = 0;
  this->_->wavebuf.lpData = reinterpret_cast<char*>(const_cast<uint8_t*>(pcm_data));

  MMRESULT result =
      waveOutPrepareHeader(this->_->hout, &this->_->wavebuf, sizeof(this->_->wavebuf));
  if (result != MMSYSERR_NOERROR) {
    MMError("waveOutPrepareHeader()", result);
    return;
  }

  LOG("Playing buffer of [" + to_string(len) + "] bytes.");

  waveOutWrite(this->_->hout, &this->_->wavebuf, sizeof(this->_->wavebuf));
}

AudioRenderer::AudioRenderer() {
  this->_ = new AudioRendererPrivate();
}

AudioRenderer::~AudioRenderer() {
  if (this->_->hout) {
    waveOutUnprepareHeader(this->_->hout, &this->_->wavebuf, sizeof(this->_->wavebuf));
    waveOutClose(this->_->hout);
  }

  delete this->_;
}
