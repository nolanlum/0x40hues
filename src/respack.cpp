#include <string>

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
  string song_xml_filename = this->base_path + "songs.xml";
  xml_document doc;
  xml_parse_result parse_result = doc.load_file(song_xml_filename.c_str());

  if (parse_result.status != xml_parse_status::status_ok) {
    ERROR("Could not parse [songs.xml]: " + string(parse_result.description()));
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
    ERROR("Could not parse [images.xml]: " + string(parse_result.description()));
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
png_byte* ImageResource::ReadAndDecode(int *width, int *height, int *color_type) {
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
    ERROR("ImageResource [" + file_name + "] was not a PNG.");
    goto DECODE_CLOSE_FILE;
  }

  // Allocate libpng structs.
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    ERROR("png_create_read_struct failed.");
    goto DECODE_CLOSE_FILE;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    ERROR("png_create_info_struct failed.");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }
  endinfo_ptr = png_create_info_struct(png_ptr);
  if (!endinfo_ptr) {
    ERROR("png_create_info_struct failed.");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }

  // The code in this if statement gets called if libpng encounters an error.
  if (setjmp(png_jmpbuf(png_ptr))) {
    ERROR("libpng returned error.");
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
  if (color_type) {
    *color_type = temp_color_type;
  }

  if (bit_depth != 8) {
    ERROR("Encountered unsupported bit depth [" + to_string(bit_depth) + "].");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }

  // Allocate memory for the image; read more PNG info first though.
  png_read_update_info(png_ptr, info_ptr);

  // glTexImage2d requires rows to be 4-byte aligned
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  rowbytes += 3 - ((rowbytes - 1) % 4);

  // We need two representations of the image data -- a block for OpenGL, and rows for libpng.
  image_data = (png_byte *) malloc(rowbytes * temp_height * sizeof(png_byte) + 15);
  row_pointers = (png_byte **) malloc(temp_height * sizeof(png_byte *));
  if (!image_data || !row_pointers) {
    if (row_pointers) {
      free(row_pointers);
    }
    ERROR("Could not allocate memory!");
    goto DECODE_DESTROY_PNG_STRUCTS;
  }

  // Set the individual row_pointers to point at the correct offsets of image_data.
  for (unsigned int i = 0; i < temp_height; i++) {
    row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
  }

  // Read the PNG data and return.
  png_read_image(png_ptr, row_pointers);

  free(row_pointers);
DECODE_DESTROY_PNG_STRUCTS:
  png_destroy_read_struct(&png_ptr, &info_ptr, &endinfo_ptr);
DECODE_CLOSE_FILE:
  fclose(fp);
DECODE_RETURN:
  return image_data;
}
