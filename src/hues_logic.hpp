#ifndef HUES_HUES_LOGIC_H_
#define HUES_HUES_LOGIC_H_

#include <ctime>

#include <audio_renderer.hpp>
#include <common.hpp>
#include <respack.hpp>
#include <video_renderer.hpp>

class HuesLogic {
  DISALLOW_COPY_AND_ASSIGN(HuesLogic)

  public:

    HuesLogic() {}
    ~HuesLogic() {}

    /**
     * Tries a (hard-coded) list of common respack names/locations and loads the first one it finds.
     *
     * This is a terrible method but whatever.
     *
     * @return whether or not the loading was successful.
     */
    bool TryLoadRespack();

    /**
     * Initializes the video renderer and (synchronously) loads all the respack images into texture
     * memory.
     */
    void InitDisplay();

    /**
     * Plays the given song. This method never returns. Unless the song name is invalid.
     *
     * @param song_list the _title_ of the song to play.
     */
    void PlaySong(const string& song_title);

  private:

    bool TryLoadRespack(const string& respack_path);

    /**
     * Play and animate one iteration of the current song, blocking to ensure the previous song has
     * played completely.
     */
    void SongLoop(const AudioResource& song, const AudioResource::Type song_type);

    static void* VideoRendererEntryPoint(void *_this);

    clock_t next_beat_ok = 0;

    ResourcePack *respack;
    AudioRenderer *a;
    VideoRenderer *v;
    pthread_t v_thread;

};

#endif // HUES_HUES_LOGIC_H_
