cmake_minimum_required(VERSION 3.16)
project(grokloop)

set(CMAKE_CXX_STANDARD 17)

add_library(event_loop event_loop/selector.cpp event_loop/selector.hpp
        event_loop/event_loop.cpp event_loop/event_loop.hpp)
add_library(opengrok opengrok/opengrok.cpp opengrok/opengrok.hpp
        opengrok/glog.cpp opengrok/glog.hpp opengrok/types.h opengrok/interfaces.hpp opengrok/id_pool.hpp
        opengrok/opengrok_proxy.cpp opengrok/opengrok_proxy.hpp)
add_library(client_observer client/client_observer.cpp client/client_observer.hpp
        client/client_proxy.cpp client/client_proxy.hpp)

add_executable(opengrok_server main.cpp)
add_executable(opengrok_client client.cpp)

include_directories(event_loop/)

target_link_libraries(opengrok event_loop)
target_link_libraries(opengrok_server opengrok)
target_link_libraries(opengrok_client opengrok client_observer)


