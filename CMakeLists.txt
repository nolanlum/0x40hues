PROJECT(0x40HUES)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# For optimized builds.
OPTION(RELEASE "Build a release executable (i.e. no debug info, optimized binary)" OFF)
IF (RELEASE)
    add_compile_options(-O2)
ELSE (RELEASE)
    add_compile_options(-ggdb)
ENDIF (RELEASE)

add_compile_options(-Wall)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# For platform-specific library checks.
INCLUDE(ConfigureChecks.cmake)

ADD_SUBDIRECTORY(src)
