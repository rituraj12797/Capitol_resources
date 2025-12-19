cmake_minimum_required(VERSION 3.10)

project(hello VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(hello source/main.cpp)