#include<iostream>
#include<thread>
void func() {
    std::cout<<" Hello from thread \n";
}

int main() {

    std::thread t1(func);
    t1.join();

    std::cout<<" finished execution \n";

}