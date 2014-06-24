INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)

# Only build statically on Windows.
IF(WIN32)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ELSE(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(WIN32)

# Ignore depreciation warnings on OSX because GLUT is deprecated.
IF(APPLE)
    add_compile_options(-Wno-deprecated)
ENDIF(APPLE)

# Find required libraries.
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)

# Check for some include files.
check_include_files(dirent.h HAVE_DIRENT_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
