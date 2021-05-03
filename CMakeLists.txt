cmake_minimum_required(VERSION 3.13)
project(opengrok C)

set(CMAKE_C_STANDARD 11)
set(OPENGROK_LIB_SRC src/lib/map.h src/lib/map.c
        src/lib/id_pool.h src/lib/queue.h
        src/lib/vector.h)
set(OPENGROK_WINDEPS wsock32 ws2_32)

if (WIN32)
    message("Using wepoll as epoll in event loop")

    add_library(opengrok_lib ${OPENGROK_LIB_SRC}
            src/lib/wepoll.c src/lib/wepoll.h)
    target_link_libraries(opengrok_lib ${OPENGROK_WINDEPS})
ELSE()
    message("Using linux epoll in event loop")

    add_library(opengrok_lib ${OPENGROK_LIB_SRC})
ENDIF()

add_library(opengrok_event_loop src/event_loop/event_loop.c
        src/event_loop/event_loop.h)

target_link_libraries(opengrok_event_loop opengrok_lib)

add_executable(opengrok main.c src/server/server.c src/server/server.h)

if (WIN32)
    message("Target OS: Windows")

    target_link_libraries(opengrok opengrok_event_loop ${OPENGROK_WINDEPS})
ELSE()
    target_link_libraries(opengrok opengrok_event_loop)
ENDIF()

include_directories(opengrok src)
add_definitions(-g)
