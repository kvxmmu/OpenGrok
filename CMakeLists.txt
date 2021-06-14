cmake_minimum_required(VERSION 3.20)
project(grokpp)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    message("using wepoll as epoll")
    set(EPOLL_SRC src/loop/wepoll.c src/loop/wepoll.h)
else()
    set(EPOLL_SRC "")
endif()

add_library(event_loop src/loop/selector.cpp src/loop/selector.hpp
        src/loop/loop.cpp src/loop/loop.hpp ${EPOLL_SRC})
add_library(server src/grokserver/server.cpp src/grokserver/server.hpp
        src/grokserver/proxy_server.cpp src/grokserver/proxy_server.hpp)
add_library(client src/grokclient/client.cpp src/grokclient/client.hpp
        src/grokclient/proxy_client.cpp src/grokclient/proxy_client.hpp)

include_directories(src/)

add_executable(grokpp main.cpp)
add_executable(grokclient client.cpp)

target_link_libraries(server event_loop zstd)
target_link_libraries(grokpp event_loop server)
target_link_libraries(grokclient event_loop client zstd)

