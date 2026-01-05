#include<iostream>
#include<thread>

void func() {
    // for function whihc are callable inside a thread use this_thread to get information regarding this process thread

    std::cout<<" Hello from thread ID : "<<std::this_thread::get_id()<<"\n";;
}


int main() {

    // this is how we launch proces threads in C++ using threads library
    std::thread t1(func);
    if(t1.joinable()){
        t1.join();
    }

    std::thread t2(func);
    if(t2.joinable()) {
        t2.join();
    }

    int n = 3;
    // defining threads with lambda function as callable 
    std::thread t([](int n) {
        std::cout<<" this is the number printer function : "<<n<<"\n";
        for(int i = 0 ; i < 10; i++) {
            std::cout<<i<<"\n";
        }
    }, n); //
    t.join();


    
    std::cout<<" runninh yet ? \n";




    std::cout<<" main thread will wait for the joint threads to finish \n";

    return 0;
}

// usually joining two non-main threads is risky as it may lead to race condition or logic errors, i.e why we generally attach all threads to the main thread

// detachign a thread(Y) is bascially detaching it from the thread to whihc it was connected ( say x) and now x won't wait for that thread(y) to finish before it(x) finishes, and y will run in background.



