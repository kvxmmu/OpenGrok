cmake_minimum_required(VERSION 3.16)
project(grokloop)

set(CMAKE_CXX_STANDARD 17)

add_library(event_loop event_loop/selector.cpp event_loop/selector.hpp
        event_loop/event_loop.cpp event_loop/event_loop.hpp)
add_library(opengrok opengrok/opengrok.cpp opengrok/opengrok.hpp
        opengrok/glog.cpp opengrok/glog.hpp opengrok/types.h opengrok/interfaces.hpp opengrok/id_pool.hpp)

add_executable(grokloop main.cpp)

include_directories(event_loop/)

target_link_libraries(opengrok event_loop)
target_link_libraries(grokloop opengrok)
