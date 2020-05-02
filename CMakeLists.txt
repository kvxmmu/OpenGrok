cmake_minimum_required(VERSION 3.16)
project(opengrok)

set(CMAKE_CXX_STANDARD 17)

add_library(binary grok/bin.cpp grok/bin.hpp)
add_library(event_loop grok/loop.cpp grok/loop.hpp grok/def.hpp)
add_library(monitor_server grok/server.cpp grok/server.hpp)
add_library(utils grok/utils.cpp grok/utils.hpp)

add_executable(server main.cpp)
add_executable(client client.cpp)

target_link_libraries(client utils binary event_loop)

target_link_libraries(event_loop utils)
target_link_libraries(utils binary)
target_link_libraries(monitor_server binary event_loop utils)

target_link_libraries(server monitor_server)

include_directories(.)

add_definitions(-g)