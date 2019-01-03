cmake_minimum_required(VERSION 3.5.1)
project(vnn VERSION 0.5)
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 11)


#main library, main & tests
find_package(PkgConfig)


find_package(GObject REQUIRED)
include_directories(${GOBJECT_INCLUDE_DIR})
message(STATUS "GObject include dir = ${GOBJECT_INCLUDE_DIR} ")
message(STATUS "GObject library = ${GOBJECT_LIBRARIES} ")



#pkg_check_modules(GST REQUIRED gstreamer-1.0)
find_package(GStreamer REQUIRED)
include_directories(${GSTREAMER_INCLUDE_DIR})
message(STATUS "Gstreamer include dir = ${GSTREAMER_INCLUDE_DIR} ")
message(STATUS "Gstreamer library = ${GSTREAMER_LIBRARY} ")

if (USE_TX2)
	include_directories("/usr/lib/aarch64-linux-gnu/glib-2.0/include/")
	include_directories("/usr/lib/aarch64-linux-gnu/gstreamer-1.0/include/")
else()
	include_directories("/usr/lib/x86_64-linux-gnu/glib-2.0/include/")
	include_directories("/usr/lib/x86_64-linux-gnu/gstreamer-1.0/include/")
endif()

find_package( OpenCV REQUIRED )

set (GSTREAMER_APP_LIB_DEPS -lgstapp-1.0 -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lgstpbutils-1.0)
#set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -fPIC -std=c++11 $(pkg-config --cflags --libs gstreamer-app-1.0)")

include_directories ("${PROJECT_SOURCE_DIR}/src")
add_subdirectory (src)
add_subdirectory (examples)

configure_file(vnn.pc.in vnn.pc @ONLY)

