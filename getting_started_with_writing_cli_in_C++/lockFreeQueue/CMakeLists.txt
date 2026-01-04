cmake_minimum_required(VERSION 3.10)

project(lockfreequeue VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Threads REQUIRED)

add_executable(lockfreequeue source/main.cpp)

target_link_libraries(lockfreequeue PRIVATE Threads::Threads)