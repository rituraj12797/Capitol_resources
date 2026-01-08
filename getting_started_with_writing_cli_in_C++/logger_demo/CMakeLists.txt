cmake_minimum_required(VERSION 3.10)

project(logger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Threads REQUIRED)

add_executable(logger app/main.cpp)

target_include_directories(logger PRIVATE ${CMAKE_SOURCE_DIR}/core/include)

target_link_libraries(logger PRIVATE Threads::Threads)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(logger PRIVATE
        -O3             # Optimization Level 3 (Aggressive speed optimization)
        -march=native   # Generate code strictly for YOUR specific CPU (enables AVX/SIMD)
        -Wall -Wextra   # Show all warnings (Catch bugs early)
    )
endif()


