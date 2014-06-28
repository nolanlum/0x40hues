# 0x40hues

Native port of the 0x40hues flash.

## Building

    mkdir build
    cd build
    cmake ..
    make

## Developing

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
