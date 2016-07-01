cmake_minimum_required(VERSION 2.8.12)
project(WiimotePlugin)

# in the CMake GUI or command line.
find_package(osvr REQUIRED)

set(BUILD_WIIUSE_SHARED_LIB OFF CACHE BOOL "Should we build as a shared library (dll/so)?")
add_definitions("-DWIIUSE_STATIC")
set(BUILD_EXAMPLE OFF CACHE BOOL "Should we build the example app?")
set(BUILD_EXAMPLE_SDL OFF CACHE BOOL "Should we build the SDL-based example app?")
set(INSTALL_EXAMPLES OFF CACHE BOOL "Should we install the example apps?")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/wiiuse/cmake")

add_subdirectory("vendor/wiiuse")

osvr_convert_json(je_nourish_wiimote_json
    je_nourish_wiimote.json
    "${CMAKE_CURRENT_BINARY_DIR}/je_nourish_wiimote_json.h")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vendor/wiiuse/src")

osvr_add_plugin(NAME je_nourish_wiimote
    CPP # indicates we'd like to use the C++ wrapper
    SOURCES
    je_nourish_wiimote.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/je_nourish_wiimote_json.h")

# If you use other libraries, find them and add a line like:
target_link_libraries(je_nourish_wiimote wiiuse)