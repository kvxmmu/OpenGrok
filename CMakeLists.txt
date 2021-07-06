cmake_minimum_required(VERSION 3.13.4)
project(grokpp)

set(CMAKE_CXX_STANDARD 17)

if(MSVC OR MSYS OR MINGW OR WIN32)
    message("-- using wepoll as epoll")
    set(EPOLL_SRC ${CMAKE_SOURCE_DIR}/src/loop/wepoll.c ${CMAKE_SOURCE_DIR}/src/loop/wepoll.h)
else()
    set(EPOLL_SRC "")
endif()

if (MSVC OR MSYS OR MINGW OR WIN32)
    set(ZSTD_LINK "${CMAKE_SOURCE_DIR}/win/libzstd.so.1.5.0")
else()
    set(ZSTD_LINK "zstd")
endif()

# add_library(hincl )  # HTTP server
add_library(event_loop src/loop/selector.cpp src/loop/selector.hpp
        src/loop/loop.cpp src/loop/loop.hpp ${EPOLL_SRC})
add_library(server src/grokserver/server.cpp src/grokserver/server.hpp
        src/grokserver/proxy_server.cpp src/grokserver/proxy_server.hpp
        src/grokserver/http_interface.cpp src/grokserver/http_interface.hpp

        src/hincl/api.c
        src/hincl/http.c src/hincl/llhttp.c src/hincl/llhttp.h
        src/hincl/hincl.cpp src/hincl/hincl.hpp)

include_directories(src/)

add_executable(grokpp main.cpp)

if (MSVC OR MSYS OR MINGW OR WIN32)
    target_link_libraries(server event_loop ${ZSTD_LINK} wsock32 ws2_32)
else()
    target_link_libraries(server event_loop ${ZSTD_LINK})
endif()

target_link_libraries(grokpp event_loop server)

if (MSVC OR MSYS OR MINGW OR WIN32)
    target_link_libraries(grokpp -static)
endif()

add_definitions(-Ofast)
