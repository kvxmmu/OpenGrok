cmake_minimum_required(VERSION 3.16)
project(opengrok)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 11)

pkg_check_modules(LIBCONFIGPP REQUIRED IMPORTED_TARGET libconfig++)

add_library(cameleo_lib src/cameleo/selector.cpp src/cameleo/selector.hpp
        src/cameleo/loop.cpp src/cameleo/loop.hpp)

add_library(grokbuffer src/grokbuffer/wrapper.cpp
        src/grokbuffer/wrapper.hpp)

include_directories(src/)

add_executable(server main.cpp src/opengrok/server/server.cpp
        src/opengrok/server/server.hpp src/opengrok/server/proxy_server.cpp
        src/opengrok/server/proxy_server.hpp)
add_executable(client client.cpp src/opengrok/client/client.cpp
        src/opengrok/client/client.hpp src/opengrok/client/proxy_client.cpp
        src/opengrok/client/proxy_client.hpp)

target_link_libraries(grokbuffer cameleo_lib PkgConfig::LIBCONFIGPP)
target_link_libraries(server grokbuffer)
target_link_libraries(client grokbuffer)


