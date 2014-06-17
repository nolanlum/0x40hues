INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)

check_include_file(windows.h HAVE_WINDOWS_H)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})