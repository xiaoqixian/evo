cmake_minimum_required(VERSION 3.16)
project(evo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-18)

set(EVO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(EVO_THIRD_PARTY_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/third_party/fmt/include
)

file(GLOB evo_src "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB evo_coro_src "${PROJECT_SOURCE_DIR}/src/coroutine/*.cpp")

include_directories(evo PUBLIC ${EVO_INCLUDE_DIR} ${EVO_THIRD_PARTY_INCLUDE_DIR})

add_subdirectory(third_party)
add_subdirectory(test)

add_library(
  evo
  SHARED
  ${evo_src}
  ${evo_coro_src}
)
