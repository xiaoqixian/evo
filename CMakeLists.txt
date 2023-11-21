cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

add_library(evo INTERFACE)

target_include_directories(evo INTERFACE include)

add_executable(test test/test1.cpp)

target_link_libraries(test evo)
