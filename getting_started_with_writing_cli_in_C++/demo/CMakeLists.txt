cmake_minimum_required(VERSION 3.10)

project(hello VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# 1. Find the threading library on the system (POSIX/Win32)
find_package(Threads REQUIRED) 

add_executable(hello source/main.cpp)

# 2. Link the threading library to your executable
target_link_libraries(hello PRIVATE Threads::Threads)