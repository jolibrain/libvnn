cmake_minimum_required(VERSION 2.8.8)
project(vnn C CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)



#main library, main & tests
find_package(PkgConfig)


find_package(GObject REQUIRED)
include_directories(${GOBJECT_INCLUDE_DIR})
include_directories("/usr/lib/x86_64-linux-gnu/glib-2.0/include/")
message(STATUS "GObject include dir = ${GOBJECT_INCLUDE_DIR} ")
message(STATUS "GObject library = ${GOBJECT_LIBRARIES} ")



#pkg_check_modules(GST REQUIRED gstreamer-1.0)
find_package(GStreamer REQUIRED)
include_directories(${GSTREAMER_INCLUDE_DIR})
message(STATUS "Gstreamer include dir = ${GSTREAMER_INCLUDE_DIR} ")
message(STATUS "Gstreamer library = ${GSTREAMER_LIBRARY} ")

set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -fPIC -std=c++11
 $(pkg-config --cflags --libs gstreamer-app-1.0)")

include_directories ("${PROJECT_SOURCE_DIR}/src")
add_subdirectory (src)
add_subdirectory (examples)

