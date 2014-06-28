set(HEADERS glew.h glxew.h wglew.h)
set(SOURCES ${HEADERS} glew.c)

add_library(glew STATIC ${SOURCES})
