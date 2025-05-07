cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)
# file(READ "/etc/os-release" OS_RELEASE_CONTENT)
# if(OS_RELEASE_CONTENT MATCHES "Ubuntu")
#   message("This is an Ubuntu system")
# else()
#   message("This is not an Ubuntu system")
# endif()

project(evo)

set(CMAKE_CXX_STANDARD 20)

set(EVO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(EVO_THIRD_PARTY_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/third_party/fmt/include
)

file(GLOB_RECURSE evo_src "${PROJECT_SOURCE_DIR}/src/*.cpp")

include_directories(${EVO_INCLUDE_DIR} ${EVO_THIRD_PARTY_INCLUDE_DIR})

add_subdirectory(third_party)
add_subdirectory(test)

add_library(
  evo
  SHARED
  ${evo_src}
)
