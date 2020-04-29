cmake_minimum_required(VERSION 3.16)
project(opengrok)

set(CMAKE_CXX_STANDARD 17)

add_library(epoll_selector grok/selector.cpp grok/selector.hpp)
add_library(opengrok_server grok/binary.cpp grok/binary.hpp
        grok/server.cpp grok/server.hpp grok/evloop.cpp grok/evloop.hpp)

add_executable(opengrok main.cpp)
add_executable(grokclient client.cpp)

target_link_libraries(opengrok_server epoll_selector)

include_directories(.)

target_link_libraries(opengrok opengrok_server)
target_link_libraries(grokclient opengrok_server)

add_definitions(-g)
