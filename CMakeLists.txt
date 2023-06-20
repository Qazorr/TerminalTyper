cmake_minimum_required(VERSION 3.16)
project(TerminalTyper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TerminalTyper main.cpp)

add_subdirectory(src)

target_link_libraries(TerminalTyper PUBLIC TerminalTyperLib)

target_include_directories(TerminalTyper PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )