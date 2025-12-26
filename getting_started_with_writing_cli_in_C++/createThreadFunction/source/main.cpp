#include<iostream>
#include<thread>
#include<chrono>
#include<atomic>
#include<windows.h> // for setting affinity on windows 


void func(int a, int b, std::string& name) {
    std::cout<<" This thread is : "<<name<<" with the id is : "<<std::this_thread::get_id<<"\n";
    std::cout<<" \n RESULT : "<<a+b<<"\n";
}


bool setThreadCoreAffinity(int core_id) {
    HANDLE current_thread = GetCurrentThread(); // return a handler to the thread which is calling this function

    // when new thread is created it will run this body hence using this handler we can reference to the new thread and not the main thread 

    // unlike linux, windows expects an affinity mask to set affinity
    // 1<<0 --> 0th core
    // 1<<2 --> 2nd core ==> 000100 ==> this process can only run on 2nd core and not on any other core ==> soft affinity
    // 1<<1 --> 1st core
    // 1<<5 --> 5th core
    

    DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << core_id);

    DWORD_PTR result= SetThreadAffinityMask(current_thread,mask);

    if(result == 0) {
        return false;
    }
    return true;
}
// this means we are definign the input arguement T whihc is the callable and the numbr and types of arguements could be any
// no except to optimize the compilation by specifying that this function won't throw any exception.
    template<typename T, typename... A> inline auto createAndStartThread(int core_id, const std::string &name, T &&func, A &&... args) noexcept {

        std::atomic<bool> running(false), failed(false);

        // pass by value here instead fo pass by reference
        /*
        * The variables name, func, args are of the main thread so they lie in the stack of main thread
        * passing them by reference tothe lambda of new thread means it will access the data through reference to the stack of main thread
        * now what may happen is that say as soon as main thread got runnign = true 
        * it return and the stack memory of this function gets destroyed so vars destroyed too. 
        * due to this now say the function whihc was passed to run inside the thread say starts it sees the values as reference to main thread's stack
        * picks up garbage from it.
        * so to make the new thread independent from maain thread in terms of data we pass these variables by values 
        * */ 



        // pass by value 
        auto thread_body = [=, &running, &failed, func = std::forward<T>(func)]() mutable {
            // WHY MUTABLE ?? 

            // Capture by Value ([=] or [var]): A copy of the variable is stored in the lambda's internal closure object as a const member. This means you cannot modify the captured copy within the lambda's body by default. Attempting to do so results in a compilation error.

            // Using mutable: To modify a variable captured by value, you must explicitly add the mutable keyword to the lambda declaration (e.g., [var]() mutable { ... }). This removes the const qualification from the closure object's function call operator, allowing the internal copy to be modified, but the changes still do not affect the original variable outside the lambda's scope.



            if(core_id >= 0 && !setThreadCoreAffinity(core_id)){ 
                // this means core affinity was nto able to setup for this thread
                std::cerr<<" Failed to set Core affinity for : "<<name<<" "<<" to "<<core_id<<"\n";
                failed = true; // assignedment failed
                return ;
            }

            std::cout<<" Core affinity set for "<<name<<" "<<" to "<<core_id<<"\n";
            running = true;

            // now that this thread is pinned we will run the function 
            func(args...); // running the  function with  arguements 
        };

        auto t = new std::thread(thread_body);

        while(!running && !failed) {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1ms); // make the main thread to sleep for 1ms  it is not clear if the initialised thread is running or has it failed ??

            // WHY this ?? ==> so that this function does not return t in an undefined state where it is not sure if this thread is running or has failed so we wait.
        }

        if(failed) {
            t->join(); // join the thread initialized, to main thread 
            delete t; // delete thread 
            t = nullptr;  // point t to nullptr
        }

        return t; // return reference to the thread;
    }



void dummyFunction(int a, int b, bool sleep) {
    std::cout<<"dummyFunction("<<a<<","<<b<<") = "<<a+b<<"\n";
    
    if(sleep) {
        using namespace std::literals::chrono_literals; // helps us naturally write 5s instead of std::chrono::second(5s)
        std::this_thread::sleep_for(5s);
    }
    std::cout<<" thread complete \n";
    return;
}

int main() {


    auto start = std::chrono::high_resolution_clock::now();
    auto t1 = createAndStartThread(1,"proc 1",dummyFunction,4,5,true);

    auto t2 = createAndStartThread(2,"proc 2",dummyFunction,4,5,true);
    // auto t3 = createAndStartThread(2,"proc 3",dummyFunction,4,5,true);

    // through running threads on different process we can achieve Kind-of true parallelism by isolating the threads to run on a fixed core.

    // tho this does nto guaruntee that other process does not un on that core and context switchitn on that core may still happen cuz other process may also take part on that core

    // also this isolation we are explicitly telling the OS to use this mask for this process means setting hard affinity.

    // to tackle this issues of context switching by other processes on this core, we use Core isolation ( done using isolcpus on linux )
    std::cout<<" waiting for threads to complete \n";

    if(t1 != nullptr) {
        t1->join();
    }

    if(t2 != nullptr) {
        t2->join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double,std::milli> duration = end-start;
    std::cout<<" main exiting , Total time : "<<(duration/1000).count();

    return 0;
    //  COMPLETE 
}

// testing 