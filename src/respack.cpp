#include <pugixml.hpp>

#include <common.hpp>
#include <filesystem.hpp>
#include <respack.hpp>

using namespace std;
using namespace pugi;

ResourcePack::~ResourcePack() {
  for (AudioResource *song : this->song_list) {
    delete song;
  }
  this->song_list.clear();

  for (ImageResource *image : this->image_list) {
    delete image;
  }
  this->image_list.clear();
}

bool ResourcePack::Init() {
  if (!FileSystem::Exists(this->base_path)) {
    ERROR("Respack base directory [" + this->base_path + "] does not exist!");
    return false;
  }

  if (!FileSystem::Exists(this->base_path + "songs.xml")) {
    ERROR("Respack doesn't contain a [songs.xml] file!");
  }
  if (!FileSystem::Exists(this->base_path + "images.xml")) {
    ERROR("Respack doesn't contain a [images.xml] file!");
  }

  LOG("Loading respack at [" + this->base_path + "].");
  this->ParseSongXmlFile();
  this->ParseImageXmlFile();

  return true;
}

void ResourcePack::ParseSongXmlFile() {
  string songXmlFileName = this->base_path + "songs.xml";
  xml_document doc;
  xml_parse_result parseResult = doc.load_file(songXmlFileName.c_str());

  if (parseResult.status != xml_parse_status::status_ok) {
    ERROR("Could not parse [songs.xml]: " + string(parseResult.description()));
    return;
  }

  xml_node root = doc.first_child();
  for (xml_node songNode : root.children()) {
    AudioResource *song = new AudioResource(
        this->base_path, songNode.attribute("name").value(),
        songNode.child("buildup").child_value());

    song->SetLoopBeatmap(songNode.child("rhythm").child_value());
    if (songNode.child("buildupRhythm")) {
      song->SetBuildupBeatmap(songNode.child("buildupRhythm").child_value());
    }
    this->song_list.push_back(song);

    DEBUG("Found song:");
    DEBUG("    Name: " + string(song->loop_name));
    DEBUG("    Buildup? " + string(song->HasBuildup() ? "Yes" : "No"));
  }

  LOG("Found [" + to_string(this->song_list.size()) + "] songs.");
}

void ResourcePack::ParseImageXmlFile() {
  string imageXmlFileName = this->base_path + "images.xml";
  xml_document doc;
  xml_parse_result parseResult = doc.load_file(imageXmlFileName.c_str());

  if (parseResult.status != xml_parse_status::status_ok) {
    ERROR("Could not parse [images.xml]: " + string(parseResult.description()));
    return;
  }

  xml_node root = doc.first_child();
  for (xml_node imageNode : root.children()) {
    ImageResource *image = new ImageResource(
        this->base_path, imageNode.attribute("name").value(),
        ImageResource::ParseAlignmentString(imageNode.child("align").child_value()));
    this->image_list.push_back(image);

    DEBUG("Found image: " + image->image_name);
  }

  LOG("Found [" + to_string(this->image_list.size()) + "] images.");
}
