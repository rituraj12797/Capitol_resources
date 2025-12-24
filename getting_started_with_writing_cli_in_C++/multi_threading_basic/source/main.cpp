#include<iostream>
#include<thread>
void func() {
    std::cout<<" Hello from thread \n";
}

int main() {

    // this is how we launch proces threads in C++ using threads library
    std::thread t1(func);
    t1.join();

    std::thread t2(func);
    t2.join();

    std::cout<<" main thread will wait for the joint threads to finish \n";

}