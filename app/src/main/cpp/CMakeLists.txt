cmake_minimum_required(VERSION 3.22.1)
project("vfscore")

# Dobby'yi GitHub'dan Çek
include(FetchContent)
FetchContent_Declare(
    Dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
)
FetchContent_MakeAvailable(Dobby)

# dobby.h dosyasının yerini C++ motorumuza öğretiyoruz
include_directories(${dobby_SOURCE_DIR}/include)

add_library(
        vfscore
        SHARED
        vfs_hook.cpp)

find_library(
        log-lib
        log)

# vfscore kütüphanemizi dobby ve log ile bağlıyoruz
target_link_libraries(
        vfscore
        PRIVATE
        dobby
        ${log-lib})
