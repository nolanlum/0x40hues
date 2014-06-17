include_directories(${0x40HUES_SOURCE_DIR}/src)
link_directories(${0x40HUES_BINARY_DIR}/src)

SET(HUES_SOURCES
    "main.cpp")

add_executable(0x40hues ${HUES_SOURCES})
target_link_libraries(0x40hues ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
