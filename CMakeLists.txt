cmake_minimum_required(VERSION 3.27)
project(evo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

# set(libevent_path "/usr/local/Cellar/libevent/2.1.12_1")
# set(libev_path "/usr/local/Cellar/libev/4.33")

file(GLOB evo_src "src/*.cpp")
file(GLOB evo_coro_src "src/coroutine/*.cpp")
add_library(evo SHARED ${evo_src} ${evo_coro_src})

target_include_directories(evo PUBLIC include)
# target_include_directories(evo INTERFACE include ${libevent_path}/include)
# target_include_directories(evo INTERFACE include ${libev_path}/include)

add_executable(out test/test1.cpp)

# target_link_libraries(evo INTERFACE ${libev_path}/lib/libev.a)
# target_link_libraries(evo INTERFACE ${libevent_path}/lib/libev.a)
target_link_libraries(out evo)
