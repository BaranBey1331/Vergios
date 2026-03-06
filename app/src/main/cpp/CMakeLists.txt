cmake_minimum_required(VERSION 3.22.1)

project("vfscore")

add_library(
        vfscore
        SHARED
        vfs_hook.cpp)

find_library(
        log-lib
        log)

target_link_libraries(
        vfscore
        ${log-lib})

