PROJECT(0x40HUES)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# Release vs debug.
set(CMAKE_CXX_FLAGS_Debug -ggdb -O0)
set(CMAKE_CXX_FLAGS_Release -O2)
add_compile_options(-Wall)

# Build statically because it's cool.
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# For platform-specific library checks.
INCLUDE(ConfigureChecks.cmake)

# Source directories go here.
ADD_SUBDIRECTORY(pugixml)
ADD_SUBDIRECTORY(src)
