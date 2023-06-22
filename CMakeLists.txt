cmake_minimum_required(VERSION 3.25)
project(cmd-script)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(cmd-script main.cpp file-watcher.cpp include/util.hpp)
