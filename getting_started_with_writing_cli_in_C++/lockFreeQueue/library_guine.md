so far we have only written codes in source/main.cpp files only 


but as our application grows larger we need to separate functions and files and to do that we use headers 

here is a simple analogy that works ===> 

consider we are making a go project 

there when we define any say data model or a helper function which we will use later we define packages for them and then imporet them wherever we need to use them, similarly here we do the same thing 

2 types 

1. if we are defining some data structures --> most probably we will be using templates here so we only create the .h file and write the complete implementation of the class, it's functions and implementation in the header file it self

2. if we are defining only some helper functions say --> runFibonacci we define the function's existance in the .h file and we defien their actual logic in a separet .cpp file 


MEANS : 

If you write a Class that is NOT a template (e.g., a specific LogManager class), you usually treat it like Scenario 2.
Also for simpler cases where we are definign inline function defintions we use header (.h) definition only
Template Class (LFQueue<T>): Header-Only (Scenario 1).

Concrete Class (LogManager): Split it!

Logger.h: class LogManager { void log(string msg); };

Logger.cpp: void LogManager::log(string msg) { ... }





The good practice in terms of folder structure for this is to 
keep the header files in separet directory ->(include) and the definition files(.cpp) into different directories anf from now on we will follow the following folder structure 


MyProject/
  |
  |___ CMakeLists.txt         <-- The Master Controller
  |
  |___ core/                  <-- YOUR LIBRARY (The "Package")
  |      |___ include/
  |      |      |___ lf_queue.h
  |      |___ src/
  |             |___ some_helper.cpp
  |
  |____ app/                   <-- YOUR EXECUTABLE (The Consumer)
  |       |___ main.cpp
  |
  |___ build/
