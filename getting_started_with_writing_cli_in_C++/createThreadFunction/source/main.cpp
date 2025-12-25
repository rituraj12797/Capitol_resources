#include<iostream>
#include<thread>
#include<chrono>
#include<atomic>


void func(int a, int b, std::string& name) {
    std::cout<<" This thread is : "<<name<<" with the id is : "<<std::this_thread::get_id<<"\n";
    std::cout<<" \n RESULT : "<<a+b<<"\n";
}

// this means we are definign the input arguement T whihc is the callable and the numbr and types of arguements could be any
// no except to optimize the compilation by specifying that this function won't throw any exception.
    template<typename T, typename... A> inline auto createAndStartThread(int core_id, const std::string &name, T &&func, A &&... args) noexcept {

        std::atomic<bool> running(false), failed(false);

        auto thread_body = [&]{
            if(core_id >= 0 && !setThreadCoreAffinity(core_id){ 
                // this means core affinity was nto able to setup for this thread
                std::cerr<<" Failed to set Core affinity for : "<<name<<" "<<pthread_self()<<" to "<<core_id<<"\n";
                failed = true; // assignedment failed
                return 
            }

            std::cout<<" Core affinity set for "<<name<<" "<<pthread_self()<<" to "<<core_id<<"\n";
            running = true;

            // now that this thread is pinned we will run the function 
            std::forward<T>(func) ((std::forward<A>(args))...); // running the template function with templates arguements 
        }

        auto t = new std::thread(thread_body);

        while(!running && !failed) {
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

bool setThreadCoreAffinity(int core_id) {

}

int main() {

}