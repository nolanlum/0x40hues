#ifndef HUES_RESPACK_H_
#define HUES_RESPACK_H_

#include <string>
#include <vector>

#include <common.h>

using namespace std;

class ImageResource;
class AudioResource;

class ResourcePack {
  DISALLOW_COPY_AND_ASSIGN(ResourcePack)

public:

  /**
   * Create a new ResourcePack whose base directory is path.
   *
   * @param path the root path of the resource pack.
   */
  ResourcePack(const string& path) : base_path(path) { }

  /**
   * Initialize the ResourcePack, parsing all XML metadata and making the List* methods callable.
   *
   * @return <code>true</code> if initialization was successful, <code>false</code> otherwise.
   */
  bool Init();

  /**
   * Populate a vector with a list of loops (including buildups) present in this resource pack.
   *
   * Note that buildups are NOT counted as a separate song, but instead are returned with (and
   * loaded with) the main loop.
   *
   * @param song_list a vector to place the song list into.
   * @return the number of songs present in this resource pack.
   */
  int ListSongs(vector<string>* const song_list) const;
  /**
   * Populate a vector with a list of images present in this resource pack.
   *
   * @param images_list a vector to place the images list into.
   * @return the number of images present in this resource pack.
   */
  int ListImages(vector<string>* const image_list) const;

  string GetBasePath() const;
  ImageResource* GetImageResource(const string& image_name) const;
  AudioResource* GetAudioResource(const string& loop_name) const;

private:
  const string base_path;
};

class ImageResource {
  DISALLOW_COPY_AND_ASSIGN(ImageResource)

  friend class ResourcePack;

public:

  /**
   * Constructs a new, named ImageResource.
   *
   * @param base_path the base path of the containing ResourcePack.
   * @param name the name of this ImageResource.
   */
  ImageResource(const string& base_path, const string& name) :
      base_path(base_path), image_name(name) { }

  void ReadAndDecode(); // TODO(nolm): fix this signature.

private:
  const string base_path;
  const string image_name;
};

class AudioResource {
  DISALLOW_COPY_AND_ASSIGN(AudioResource)

  friend class ResourcePack;

public:

  /**
   * Constructs a new, named AudioResource.
   *
   * @param base_path the base path of the containing ResourcePack.
   * @param name the name of this AudioResource.
   */
  AudioResource(const string& base_path, const string& name) :
      base_path(base_path), loop_name(name) { }

  /** Returns whether or not this song has a buildup. */
  bool HasBuildup() const;

  string GetBuildupBeatmap() const;
  string GetLoopBeatmap() const;

private:

  void SetBuildupBeatmap(const string& beatmap);
  void SetLoopBeatmap(const string& beatmap);

  string buildup_beatmap;
  string loop_beatmap;

  const string base_path;
  const string loop_name;
};

#endif // HUES_RESPACK_H_
