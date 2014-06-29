#include <assert.h>

#include <fstream>
#include <string>

#include <pugixml.hpp>

#include <audio_decoder.hpp>
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
    ERR("Respack base directory [" + this->base_path + "] does not exist!");
    return false;
  }

  if (!FileSystem::Exists(this->base_path + "songs.xml")) {
    ERR("Respack doesn't contain a [songs.xml] file!");
  }
  if (!FileSystem::Exists(this->base_path + "images.xml")) {
    ERR("Respack doesn't contain a [images.xml] file!");
  }

  LOG("Loading respack at [" + this->base_path + "].");
  this->ParseSongXmlFile();
  this->ParseImageXmlFile();

  return true;
}

void ResourcePack::ParseSongXmlFile() {
  string song_xml_filename = this->base_path + "songs.xml";
  xml_document doc;
  xml_parse_result parse_result = doc.load_file(song_xml_filename.c_str());

  if (parse_result.status != xml_parse_status::status_ok) {
    ERR("Could not parse [songs.xml]: " + string(parse_result.description()));
    return;
  }

  xml_node root = doc.first_child();
  for (xml_node song_node : root.children()) {
    AudioResource *song = new AudioResource(
        this->base_path, song_node.child("title").child_value(),
        song_node.attribute("name").value(), song_node.child("buildup").child_value());

    song->SetLoopBeatmap(song_node.child("rhythm").child_value());
    if (song_node.child("buildupRhythm")) {
      song->SetBuildupBeatmap(song_node.child("buildupRhythm").child_value());
    }
    this->song_list.push_back(song);

    DEBUG("Found song:");
    DEBUG("    Title: " + string(song->song_title));
    DEBUG("    Buildup? " + string(song->HasBuildup() ? "Yes" : "No"));
  }

  LOG("Found [" + to_string(this->song_list.size()) + "] songs.");
}

void ResourcePack::ParseImageXmlFile() {
  string image_xml_filename = this->base_path + "images.xml";
  xml_document doc;
  xml_parse_result parse_result = doc.load_file(image_xml_filename.c_str());

  if (parse_result.status != xml_parse_status::status_ok) {
    ERR("Could not parse [images.xml]: " + string(parse_result.description()));
    return;
  }

  xml_node root = doc.first_child();
  for (xml_node image_node : root.children()) {
    ImageResource *image = new ImageResource(
        this->base_path, image_node.attribute("name").value(),
        ImageResource::ParseAlignmentString(image_node.child("align").child_value()));
    this->image_list.push_back(image);

    DEBUG("Found image: " + image->image_name);
  }

  LOG("Found [" + to_string(this->image_list.size()) + "] images.");
}

int ResourcePack::GetAllSongs(vector<AudioResource*>& song_list) const {
  song_list.insert(song_list.end(), this->song_list.begin(), this->song_list.end());
  return this->song_list.size();
}

int ResourcePack::GetAllImages(vector<ImageResource*>& image_list) const {
  image_list.insert(image_list.end(), this->image_list.begin(), this->image_list.end());
  return this->image_list.size();
}

// =====================================================================
//                     I m a g e R e s o u r c e
// =====================================================================

// Borrowed from https://github.com/DavidEGrayson/ahrs-visualizer/blob/master/png_texture.cpp
png_byte* ImageResource::ReadAndDecode(int *width, int *height, int *color_type) const {
  png_byte header[8];
  png_byte *image_data = NULL;
  png_byte **row_pointers = NULL;

  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL, endinfo_ptr = NULL;

  int rowbytes = 0;

  // Open the file.
  string file_name = this->base_path + "/Images/" + this->image_name + ".png";
  FILE *fp = fopen(file_name.c_str(), "rb");
  if (!fp) {
    perror(file_name.c_str());
    goto DECODE_RETURN;
  }

  // Read the PNG header, as recommended by libpng.
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    ERR("ImageResource [" + file_name + "] was not a PNG.");
    goto DECODE_CLOSE_FILE;
  }

  // Allocate libpng structs.
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    ERR("png_create_read_struct failed.");
    goto DECODE_CLOSE_FILE;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    ERR("png_create_info_struct failed.");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }
  endinfo_ptr = png_create_info_struct(png_ptr);
  if (!endinfo_ptr) {
    ERR("png_create_info_struct failed.");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }

  // The code in this if statement gets called if libpng encounters an error.
  if (setjmp(png_jmpbuf(png_ptr))) {
    ERR("libpng returned error.");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }

  // Read png metadata.
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  int bit_depth, temp_color_type;
  png_uint_32 temp_width, temp_height;
  png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &temp_color_type,
      NULL, NULL, NULL);
  if (width) {
    *width = temp_width;
  }
  if (height) {
    *height = temp_height;
  }

  // Transform PNG images into 8bpc GA.
  png_set_expand(png_ptr);
  png_set_packing(png_ptr);

  // Allocate memory for the image; read and update more PNG info first though.
  png_read_update_info(png_ptr, info_ptr);
  temp_color_type = png_get_color_type(png_ptr, info_ptr);
  if (color_type) {
    *color_type = temp_color_type;
  }

  // Make sure we have an expected image type.
  assert(png_get_bit_depth(png_ptr, info_ptr) == 8);
  assert((temp_color_type & PNG_COLOR_MASK_ALPHA) == PNG_COLOR_MASK_ALPHA);

  // glTexImage2d requires rows to be 4-byte aligned
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  rowbytes += 3 - ((rowbytes - 1) % 4);

  // We need two representations of the image data -- a block for OpenGL, and rows for libpng.
  image_data = new png_byte[rowbytes * temp_height * sizeof(png_byte) + 15];
  row_pointers = new png_byte*[temp_height * sizeof(png_byte *)];

  // Set the individual row_pointers to point at the correct offsets of image_data.
  for (unsigned int i = 0; i < temp_height; i++) {
    row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
  }

  // Read the PNG data and return.
  png_read_image(png_ptr, row_pointers);

  delete[] row_pointers;
DECODE_DESTROY_PNG_STRUCTS:
  png_destroy_read_struct(&png_ptr, &info_ptr, &endinfo_ptr);
DECODE_CLOSE_FILE:
  fclose(fp);
DECODE_RETURN:
  return image_data;
}

// =====================================================================
//                     A u d i o R e s o u r c e
// =====================================================================

void AudioResource::ReadAndDecode(const Type audio_type) {
  struct song_info *song = (audio_type == Type::LOOP) ? &this->loop : &this->buildup;

  string file_name = this->base_path + "/Songs/" + song->name + ".mp3";
  ifstream audio_file(file_name, ifstream::binary);

  if (audio_file) {
    audio_file.seekg (0, audio_file.end);
    int file_length = audio_file.tellg();
    audio_file.seekg (0, audio_file.beg);

    uint8_t *buffer = new uint8_t[file_length];
    audio_file.read(reinterpret_cast<char*>(buffer), file_length);

    AudioDecoder decoder(buffer, file_length);
    song->pcm_data = decoder.Decode(&song->sample_count, &song->channel_count, &song->sample_rate);

    // Calculate length of each beat. If there is no beatmap, the song is one long beat.
    if (song->beatmap.empty()) {
      song->usec_per_beat = (int) ((float) song->sample_count / song->sample_rate * 1000 * 1000);
    } else {
      song->usec_per_beat = (int) ((float) song->sample_count / song->sample_rate
          * 1000 * 1000 / song->beatmap.length());
    }

    LOG("Loaded [" + file_name + "]: " + to_string(song->beatmap.length()) + " beats at "
        + to_string(song->usec_per_beat) + " usec each.");
  } else {
    ERR("Unable to open audio file [" + file_name + "] for read!");
  }
}
