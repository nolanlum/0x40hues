#include <assert.h>
#include <unistd.h>

#include <vector>

#include <filesystem.hpp>
#include <hues_logic.hpp>

bool HuesLogic::TryLoadRespack(const string& respack_path) {
  if (FileSystem::Exists(respack_path)) {
    this->respack = new ResourcePack(respack_path);
    this->respack->Init();
    return true;
  }

  LOG("Tried respack path [" + respack_path + "].");
  return false;
}

bool HuesLogic::TryLoadRespack() {
   if (!(TryLoadRespack("respacks/Default/")
      || TryLoadRespack("../respacks/Default/")
      || TryLoadRespack("../../respacks/Default/"))) {
    ERR("Couldn't load a respack!");
    return false;
   }
   return true;
}

void HuesLogic::InitDisplay() {
  this->v = new VideoRenderer();
  pthread_create(&this->v_thread, NULL, HuesLogic::VideoRendererEntryPoint, this);
  this->v->WaitForTextureLoad();
}

void HuesLogic::PlaySong(const string& song_title) {
  vector<AudioResource*> song_list;
  this->respack->GetAllSongs(song_list);

  this->a = new AudioRenderer();
  this->a->Init(2, 44100);

  // Worst linear search in the history of ever.
  AudioResource *song = NULL;
  for (auto le_song : song_list) {
    if (le_song->GetTitle() == song_title) {
      song = le_song;
    }
  }
  if (!song) {
    ERR("Respack didn't contain requested song [" + song_title + "]!");
    return;
  }
  song->ReadAndDecode(AudioResource::Type::LOOP);

  if (song->HasBuildup()) {
    song->ReadAndDecode(AudioResource::Type::BUILDUP);
    this->SongLoop(*song, AudioResource::Type::BUILDUP);
  }

  for (;;) {
    this->SongLoop(*song, AudioResource::Type::LOOP);
  }
}

void HuesLogic::SongLoop(const AudioResource& song, const AudioResource::Type song_type) {
  const string beatmap = song.GetBeatmap(song_type).empty() ? "." : song.GetBeatmap(song_type);
  const int beat_count = !beatmap.length() ? 1 : beatmap.length();
  const int beat_length_usec = song.GetBeatDurationUsec(song_type);

  assert(song.GetChannelCount(song_type) == 2);
  assert(song.GetSampleRate(song_type) == 44100);
  a->PlayAudio(song.GetPcmData(song_type), song.GetPcmDataSize(song_type));

  for (int cur_beat = 0; cur_beat < beat_count ; cur_beat++) {
    AudioResource::Beat beat_type = AudioResource::ParseBeatCharacter(beatmap.at(cur_beat));

    while (clock() < this->next_beat_ok) {
      usleep(100);
    }

    switch (beat_type) {
      case AudioResource::Beat::NO_TRANSITION: break;
      case AudioResource::Beat::IMAGE_ONLY:
        this->v->SetImage(AudioResource::Beat::IMAGE_ONLY);
        break;
      case AudioResource::Beat::COLOR_ONLY:
        this->v->SetColor();
        break;
      default:
        this->v->SetColor();
        this->v->SetImage(beat_type);
        break;
    }

    this->next_beat_ok =
        (clock_t) ((((float) clock()) / CLOCKS_PER_SEC + (beat_length_usec / 1000.f / 1000.f))
            * CLOCKS_PER_SEC);
  }
}

void* HuesLogic::VideoRendererEntryPoint(void* hueslogic) {
  HuesLogic *_this = static_cast<HuesLogic*>(hueslogic);
  const char* fake_argv[] { "0x40hues" };

  _this->v->Init(1, const_cast<char**>(fake_argv));
  _this->v->LoadTextures(*_this->respack);
  _this->v->DoGlutLoop();

  return NULL;
}
