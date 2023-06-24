cmake_minimum_required(VERSION 3.25)
project(auto-create)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(auto-create main.cpp file-watcher.cpp)
