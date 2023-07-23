cmake_minimum_required(VERSION 3.16)
project(TerminalTyper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_CPPCHECK "cppcheck")

add_executable(TerminalTyper main.cpp)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_library(TerminalTyperLib ${SRC_FILES})

target_include_directories(TerminalTyperLib PUBLIC src)

target_link_libraries(TerminalTyper PRIVATE TerminalTyperLib)