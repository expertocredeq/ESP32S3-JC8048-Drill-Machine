cmake_minimum_required(VERSION 3.16)

# Set compile options for better optimization and warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# Include ESP-IDF project cmake file
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cnc_control_system)