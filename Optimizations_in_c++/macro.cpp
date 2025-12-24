#include<iostream>
#include<stdio.h>
#include<vector>
#include<chrono>

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)


// ASSERT function simply check if the !cond == true ==> if condition is false then logs the message and exists with an error

// WHAT IS 'noexcept' ?? ==> noexcept is a keyword we append in the function description of a function this guraruntess the compielr that this function won't throw an exception. 

//   WHY IS THIS NEEDED ?? ==> OPTIMIZATION (HOW) => normally when function are called compilers has to generate hidden code for exception handling just in case if the program ever throws an exception, by explicitly adding this keyword in the function definition we guaruntee the compler that this won't throw an exception so the overhead associated with the exception handling is reduced leading to further improvements in performance, 

// What if the program still throws an exception ?? ==> in that case the compiler won't use destructor or cleanu the things, it will just terminate the program straight up.

inline auto ASSERT(bool cond, const std::string& msg) noexcept {
        if(UNLIKELY(!cond)) {
            std::cerr<<msg<<std::endl;
            exit(EXIT_FAILURE);
        }
    }


// FATAL function simply exists with a string message we must use this when we trigger a fatal scenario and server crashes.

inline auto FATAL(const std::string& msg) noexcept {
    std::cerr<<msg<<std::endl;
    exit(EXIT_FAILURE);
}

// int arr[1000000]; // array of primes
int main() {
    std::vector<int> arr;
    std::vector<int> vis(100000002,1);
    // std::fill(std::begin(vis),std::end(vis), 1);

    for(int i = 2 ; i <= 100000000; i++) {
        if(vis[i] == 1) {
            arr.push_back(i);
            for(int j = i ; j <= 100000000; j+= i) {
                vis[j] = 0;
            }
        }
    }

    for(int i = 0 ; i < 10; i++ ) std::cout<<arr[i]<<"\n";
    int sum = 0;
    // un informed loop where CPU may gamble
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 1; i < arr.size(); i++) {
        if(arr[i] - arr[i-1] < 500) { // prime difference 
            sum += arr[i] - arr[i-1]; // add this 
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double,std::nano> duration = end - start;
    std::cout << " NORMAL Duration: " << duration.count() << " ns" << std::endl;

    // informed with LIKELY

    start = std::chrono::high_resolution_clock::now();
    for(int i = 1; i < arr.size(); i++) {
        if(LIKELY(arr[i] - arr[i-1] < 500)) { // prime difference 
            sum += arr[i] - arr[i-1]; // add this 
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << " INFORMED Duration: " << duration.count() << " ns" << std::endl;

    // misinformed with UNLIKELY so that CPU hits branch mis prediction almost all time
      start = std::chrono::high_resolution_clock::now();
    for(int i = 1; i < arr.size(); i++) {
        if(UNLIKELY(arr[i] - arr[i-1] < 500)) { // prime difference 
            sum += arr[i] - arr[i-1]; // add this 
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << " MISINFORMED Duration: " << duration.count() << " ns" << std::endl;

    if(sum > 0) std::cout<<" IGNORE THIS  : "<<sum<<"\n";
    // remember to compile with optimization level -O1 as naive compilatio does noe uses pre hints 

    /*
    RESULTS with O1 were 
     NORMAL Duration: 3.001e+006 ns
    INFORMED Duration: 3e+006 ns
    MISINFORMED Duration: 1.2011e+007 ns
    
    this is expected because this is a monotonous and tru will be evaluttaed as prime difference is < 500 most of times hence the hardware predictor mtched predicted tru most of time even in normal case where we haven't informed the compiler and thats the reason why NORMAL and INFORMED time are same 
    , moving forward to MISINFOMED ==> the compiler trusted us and we lied to it so in each iteration it had to flush back re jump and do all sort of en efficientthings to follow the correct branch after eventually falling to wrong branch due to our recommendatin each time, this elad to 4x more time.

    so if hardware predictor is that smart already when to use LIKELY then ?? 
    we use it in those places where the past program data is not available or is of no use to CPU for judging the outcome, 

    another use case for this is codl starts when there is not enough program data we may include likely in our IF statement to make those first oterations fast amd after that when program data is available the hardware predictor starts it's work.

    an exampel on this ====>    we use it in those places where the past program data is not available or is of no use to CPU for judging the outcome, 

    SAY : we created our component of process order 
    we have a loop inside which we process orders, inside this loop we check for e a very rate corrupt packer error (means this error happend ver few time) say the handler for this is very large ( 500 lines of code ), now since we knwo that this is unlikely to hapen we place UNLIKELY in the if statement, what it does is thet it moves 

    

 */ 

}