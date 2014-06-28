PROJECT(0x40HUES)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Release vs debug.
# Default build type to debug if not set
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -ggdb -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -O2")
set(CMAKE_CXX_FLAGS "-std=c++11")
add_compile_options(-Wall -Werror -DGLEW_STATIC)

# For platform-specific library checks.
INCLUDE(ConfigureChecks.cmake)

# Source directories go here.
ADD_SUBDIRECTORY(pugixml)
ADD_SUBDIRECTORY(glew)
ADD_SUBDIRECTORY(src)
