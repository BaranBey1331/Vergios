cmake_minimum_required(VERSION 3.22.1)
project("vfscore")

# Dobby Kütüphanesini otomatik çekmek için FetchContent kullanıyoruz
include(FetchContent)
FetchContent_Declare(
    Dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG master
)
FetchContent_MakeAvailable(Dobby)

add_library(
        vfscore
        SHARED
        vfs_hook.cpp)

find_library(
        log-lib
        log)

# vfscore kütüphanemizi Dobby ve log kütüphaneleriyle bağlıyoruz
target_link_libraries(
        vfscore
        PRIVATE
        Dobby
        ${log-lib})
