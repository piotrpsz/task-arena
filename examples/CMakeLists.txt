cmake_minimum_required(VERSION 3.29)
project(task_arena)

set(CMAKE_CXX_STANDARD 20)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(fmt REQUIRED)


add_executable(task_arena main.cpp
        ../pool.h
        ../safe_queue.h
        ../join_threads.h
        ../printer.h
        ../stealing_queue.h
        ../function_wrapper.h)

target_link_libraries(task_arena
        fmt::fmt
)