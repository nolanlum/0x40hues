include_directories(
    ${0x40HUES_SOURCE_DIR}/glew
    ${0x40HUES_SOURCE_DIR}/src
    ${0x40HUES_SOURCE_DIR}/pugixml
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    #{MAD_INCLUDE_DIRS}
    ${0x40HUES_BINARY_DIR})

SET(HUES_HEADERS
    "audio_decoder.hpp"
    "audio_renderer.hpp"
    "common.hpp"
    "filesystem.hpp"
    "hues_logic.hpp"
    "respack.hpp"
    "video_renderer.hpp")

SET(HUES_SOURCES
    ${HUES_HEADERS}
    "audio_decoder.cpp"
    "hues_logic.cpp"
    "main.cpp"
    "respack.cpp"
    "video_renderer.cpp")

IF(WIN32)
    SET(HUES_SOURCES
        ${HUES_SOURCES}
        "audio_renderer_win32.cpp")
ELSEIF(APPLE)
ELSEIF(UNIX)
ENDIF()

foreach(f IN LISTS HUES_SOURCES)
    get_filename_component(b ${f} NAME)
    set_source_files_properties(${f} PROPERTIES COMPILE_DEFINITIONS "__SRCFILE__=\"${b}\"")
endforeach()

add_executable(0x40hues ${HUES_SOURCES})
target_link_libraries(0x40hues ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(0x40hues ${MAD_LIBRARIES})
target_link_libraries(0x40hues ${PNG_LIBRARIES})
target_link_libraries(0x40hues ${GLUT_LIBRARIES})
target_link_libraries(0x40hues glew)
target_link_libraries(0x40hues ${OPENGL_LIBRARIES})
target_link_libraries(0x40hues pugixml)

IF(WIN32)
    set_property(TARGET 0x40hues PROPERTY LINK_SEARCH_END_STATIC ON)
    add_definitions(-DFREEGLUT_STATIC)
    target_link_libraries(0x40hues winmm)
ENDIF(WIN32)
