cmake_minimum_required(VERSION 2.8.8)
project(vnn C CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -fPIC -std=c++11")


#main library, main & tests
include_directories ("${PROJECT_SOURCE_DIR}/src")
add_subdirectory (src)
