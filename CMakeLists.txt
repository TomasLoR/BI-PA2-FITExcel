cmake_minimum_required(VERSION 3.22)
project(semester)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -g -fsanitize=address")

link_directories(${CMAKE_SOURCE_DIR}/x86_64-linux-gnu)

add_executable(semester
        all_in_one.cpp
        )

target_link_libraries(semester expression_parser)