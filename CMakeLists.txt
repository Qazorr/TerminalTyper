cmake_minimum_required(VERSION 3.16)
project(TerminalTyper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_CPPCHECK "cppcheck")

option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES "clang-tidy")
    if(CLANG_TIDY_COMMAND)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND};--fix")
    else()
        message(WARNING "clang-tidy not found. Code will not be checked.")
    endif()
endif()

add_executable(TerminalTyper main.cpp)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_library(TerminalTyperLib ${SRC_FILES})

target_include_directories(TerminalTyperLib PUBLIC src)

target_link_libraries(TerminalTyper PRIVATE TerminalTyperLib)