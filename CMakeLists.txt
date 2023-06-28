cmake_minimum_required(VERSION 3.25)
project(auto-create)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

#add_compile_definitions(AUTO_DETECT)

add_executable(auto-create main.cpp dir_item.cpp utils.cpp)
