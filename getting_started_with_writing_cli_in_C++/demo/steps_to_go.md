

1. make the project direcroty 'my_project'
2. follow the following folder structure 
            myproject/
              |________ build/
              |________ source/
              |            |_____main.cpp
              |
              |________ include/   <== headers here
              |
              |________CMakeLists.txt

3. source directory contains the main C++ file

4. Write the CMakeLists.txt file with 
    4.1 => CMake version => cmake_minimum_required(VERSION 3.10)

    4.2 => project(<NAME> VERSION <VERSION> LANGUAGES <LANGUAGES>)     => project(hello VERSION 1.0 LANGUAGES CXX)

    4.3 => set language to be used => set(CMAKE_CXX_STANDARD 17)
    4.4 => set mandatory rules for standard language => set(CMAKE_CXX_STANDARD_REQUIRED true)

    4.5 =. ad adress to the main executable file => add_executable(hello source/main.cpp)

5. Go to build folder and run 

    ```
    cmake -G "MinGW Makefiles" ..  
    ```
    this tell the machine to use MinGW as the build system which is cross platform 
6.  This reads all necessary information from the CmakeLists.txt and writes a make file for the project 

7. Then in the build directory only run 
    ```
    cmake --build .
    ```
    this tells cmake to now actually build the executible using the makefile it generated.

8. This last step creates the .exe file which could be executed as 
    ``` ./hello.exe
    ```


