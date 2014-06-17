include_directories(${0x40HUES_SOURCE_DIR}/src)
link_directories(${0x40HUES_BINARY_DIR}/src)

SET(HUES_SOURCES
    "main.cpp")

add_executable(0x40hues ${HUES_SOURCES})
target_link_libraries(0x40hues ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
set_property(TARGET 0x40hues PROPERTY LINK_SEARCH_END_STATIC ON)

IF(WIN32)
    add_definitions(-DFREEGLUT_STATIC -DGLEW_STATIC)
    target_link_libraries(0x40hues winmm)
ENDIF(WIN32)
