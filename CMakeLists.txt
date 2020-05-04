cmake_minimum_required(VERSION 3.16)
project(opengrok2)

set(CMAKE_CXX_STANDARD 17)


add_library(server_lib servers.cpp servers.hpp)
add_library(defs opengrok/def.cpp opengrok/def.hpp)
add_library(event_loop opengrok/loop.cpp opengrok/loop.hpp opengrok/packets_queue.cpp
        opengrok/packets_queue.hpp)
add_library(utils opengrok/utils.cpp opengrok/utils.hpp)

add_executable(opengrok2 main.cpp)
add_executable(grok client/client.cpp client/clients.cpp client/clients.hpp)

target_link_libraries(opengrok2 event_loop defs server_lib utils)
target_link_libraries(grok event_loop defs utils)

link_directories(opengrok/)
include_directories(.)