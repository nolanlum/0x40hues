# 0x40hues

Native port of the 0x40hues flash.

## Building

You'll need CMake since that's the build system we're using here. It's pretty standard:

    mkdir build
    cd build
    cmake ..
    make

In terms of libraries, you'll need the development versions of `OpenGL`, `libfreeglut`, `libpng`,
and `libmad`. You'll also need to have working `pthreads`, but every GCC toolchain worth its salt
should have that (I think).

### Linux

For `apt`-based distros, you should be able to bring in all the dependencies with:

    sudo apt-get install mesa-common-dev freeglut3-dev libpng1.6-dev libmad0-dev

For `rpm`-based distros, you're on your own.

### OSX

Use Homebrew. That is all.

### Windows

I personally use win_builds(+MSYS), but you can probably build with any GCC toolchain for Win32.

## Developing

Watches pelcome.

### Sublime Text 2

If you're using Sublime Text, the following settings should get you up to speed with code style.
As an added bonus, you can get SublimeClang working (it's nifty).

    "settings":
    {
      "spell_check": true,
      "tab_size": 2,
      "translate_tabs_to_spaces": true,
      "trim_trailing_white_space_on_save": true,
      "ensure_newline_at_eof_on_save": true,
      "rulers": [100],
      "sublimeclang_additional_language_options":
      {
        // For example, you can use "c++": ["-std=c++11"] to enable C++11 features.
        "c++" : ["-std=c++11"]
      },
      "sublimeclang_options":
      [
        "-I/usr/local/include",
        "-Wall",
        "-Werror",
        "-Wno-deprecated",
        "-I${folder:${project_path:README.md}}/build",
        "-I${folder:${project_path:README.md}}/glew",
        "-I${folder:${project_path:README.md}}/pugixml",
        "-I${folder:${project_path:README.md}}/src",
        "-D__SRCFILE__",
        "-D_DEBUG",

        // Clang is dumb and doesn't understand to_string
        "-Dto_string(...)=std::string(\"\")"
      ]
    }
