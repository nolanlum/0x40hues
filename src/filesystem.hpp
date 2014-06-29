#ifndef HUES_FILESYSTEM_HPP_
#define HUES_FILESYSTEM_HPP_

#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>

#include <vector>

#include <common.hpp>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace std;

namespace FileSystem {
  inline bool Exists(const string& filename) {
    struct stat buf;
    return stat(filename.c_str(), &buf) != -1;
  }

  inline void ListDirectory(const string& dir_name, vector<string>* const dir_list) {
#ifdef WIN32
    // TODO(nolm): implement me.
    #error "Not yet implemented!"
#else
    DIR *dirHandle = opendir(dir_name.c_str());
    struct dirent *dirEntry = NULL;
    while ((dirEntry = readdir(dirHandle)) != NULL) {
      if (strcmp(dirEntry->d_name, ".") || strcmp(dirEntry->d_name, "..")) {
        continue;
      }
      dir_list->push_back(string(dirEntry->d_name));
    }
    closedir(dirHandle);
#endif
  }

}

#endif // HUES_FILESYSTEM_HPP_
