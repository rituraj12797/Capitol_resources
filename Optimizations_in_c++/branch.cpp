
/*

An Example that helped me 

What does it predict? It predicts the outcome (True/False) and the location of the next instruction (Where to jump).

Does it predict which code to execute? Yes. It guesses which block (the if block or the else block) will run and starts running it immediately, effectively "gambling" on the result.

When does it check? Later in the pipeline, once the math for the condition is actually finished.

Here is exactly how this works with a simple example.

The "Pipeline" Problem
CPUs operate like a factory assembly line. While one instruction is being Calculated, the CPU wants to Fetch the next 10 instructions to keep the line moving.

But when it sees an if, it hits a fork in the road. It doesn't know which instructions to fetch next until the calculation is done. It can't wait (that would stop the assembly line), so it guesses.

The Example Scenario
Imagine this C++ code:

C++

// Step A: The condition
if (x > 50) {
    // Step B: The "Taken" path
    y = y * 2;
} else {
    // Step C: The "Not Taken" path
    y = y + 1;
}
Here is the timeline of what happens inside the CPU:

1. The Encounter (Fetch Stage)
The CPU sees the instruction: CMP x, 50 (Compare x and 50) followed by a JMP (Branch). At this exact nanosecond, the CPU does not know the value of x. The value of x might still be loading from memory (RAM), which is very slow.

2. The Prediction (The "Gamble")
The CPU looks at its "history book" (Branch Predictor) and sees that usually, this branch is True.

Prediction: x will be greater than 50.

Action: It immediately starts fetching and executing Step B (y = y * 2).

3. Speculative Execution
The CPU starts doing the work for y = y * 2 speculatively.

It loads y.

It multiplies y by 2.

Crucial Note: It does not save the final result to the permanent register yet. It keeps the result in a temporary "buffer" (Reorder Buffer). It waits here, holding the result, effectively saying: "I have the answer ready, just tell me if I should keep it."

4. The Moment of Truth (Execute Stage)
Finally, the data for x arrives from memory. The CPU executes CMP x, 50.

Scenario A: The Prediction was CORRECT

The CPU sees x was indeed 60.

It says: "Great! That work we did for y = y * 2 is valid."

The result is committed (made permanent). The pipeline never stopped; zero time was lost.

Scenario B: The Prediction was WRONG (Misprediction)

The CPU sees x was actually 10.

It says: "Stop! Everything we did for y = y * 2 is garbage."

The Penalty: It must flush the pipeline. It throws away the work for y = y * 2, goes back to the fork, and starts fetching Step C (y = y + 1) from scratch.

Why optimize this?
If the CPU guesses correctly, it feels like the branch instruction took 0 cycles (it's free). If the CPU guesses wrong, it might lose 15-20 cycles clearing the mess and restarting.

This is why we want branches to be predictable (always True or always False) rather than random (True/False/True/False)—it allows the CPU to keep the assembly line moving without flushing the work it has already done.

Next Step
Since you are interested in low-level details, would you like to know how the CPU actually "guesses"? We can look at how Saturating Counters work (it's a very simple state machine used for this).

*/



// MAINLY WE are tryign to warm the cache for both scenarios and make the branching conditionals complex enough so that the compiler do not optimize them to simple CMOV optimzed ops, testy purely on basis of branch prediction or miss prediction.



#include<bits/stdc++.h>

using namespace std;

int main() {
    auto st = chrono::high_resolution_clock::now();
    chrono::duration<double,nano> srtt = st - st;
    chrono::duration<double,nano> usrtt = st-st;
    chrono::duration<double,nano> msrtt = st-st;
    chrono::duration<double,nano> musrtt = st-st;
    int t = 1;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> size_distrib(10000, 100000); // Range 10^4 to 10^5

    // 2. Setup Vector of Test Sizes
    int num_tests = 10;
    vector<int> tests(num_tests);
    
    // FILLER LOGIC: Fill 'tests' with random sizes
    for(int &size : tests) {
        size = size_distrib(gen);
    }


   for(auto x : tests) {
     int n = x; // keep n >= 1000
    int arr[n];

    // sorted test first 
    
    for(int i = 0 ; i < n; i++) {
        arr[i] = i%(n/4);
    }
    int a = 5;
    int b = 6;
    for(int i = 10 ; i < 100000000; i++) { // throtling the OCU to wake up 
        a = (a*b)%10;
        b = (a-b)/2;
    }
    int sum = 0;
    int antisum = 0;

    
    sort(arr,arr+n); // sortign will do sorting + warm the cache with arr
    int med = arr[n/2];


    // ================== THE SORTED LOOP ================== 
    // MOSTLY CORRECT PREDICTIONS
    auto start = chrono::high_resolution_clock::now();

    // K Lop is just for increasing the number of iterations to make the test slow enough so that clock test it 
    for(int k = 0 ; k < 500; k++) {

        for(int i = 51 ; i < n - 500; i++) {
            if(arr[i + k] < med) { 
                if(arr[k] < med) {
                    sum += arr[i];
                } else {
                    sum += arr[i]/2;
                }
                
            } else {
                sum -= arr[i];
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double,nano> duration = end - start;
    // cout<<" TIME THIS LOOP TOOK : "<<" "<<duration.count()<<"\n";

    srtt += duration;

    
    
    
    // shuffle the arr 
    auto rng = default_random_engine {};
    shuffle(arr,arr+n,rng); // this warmed the cache with arr
    // ================ UNSORTED LOOP ======= mostly incorrect prediction
    start = chrono::high_resolution_clock::now();
    for(int k = 0 ; k < 500; k++) {

        for(int i = 51 ; i < n - 500; i++) {
            if(arr[i + k] < med ) { 
                 if(arr[k] < med) {
                    sum += arr[i];
                } else {
                    sum += arr[i]/2;
                }
                
            } else {
                sum -= arr[i];
            }
        }
    }


    end = chrono::high_resolution_clock::now();
    duration = end - start;

    usrtt += duration;






    // now mathematical approach 
    sort(arr,arr+n); // sortign will do sorting + warm the cache with arr
     med = arr[n/2];
    start = chrono::high_resolution_clock::now();

    // K Lop is just for increasing the number of iterations to make the test slow enough so that clock test it 
    for(int k = 0 ; k < 50; k++) {

        for(int i = 51 ; i < n - 50; i++) {

            int val = arr[i];
            
            int inner_case = (arr[k] < med ? val : (val>>1)); // division by 2 == >faster way to do by >> 1

            int term = ( arr[i+k] < med ) ? inner_case : -arr[i];
            
            sum += term; 
        }
    }

    end = chrono::high_resolution_clock::now();
    duration = end - start;
    // cout<<" TIME THIS LOOP TOOK : "<<" "<<duration.count()<<"\n";

    msrtt += duration;


    rng = default_random_engine {};
    shuffle(arr,arr+n,rng);

    start = chrono::high_resolution_clock::now();
   for(int k = 0 ; k < 50; k++) {

        for(int i = 51 ; i < n - 50; i++) {

            int val = arr[i];
            
            int inner_case = (arr[k] < med ? val : (val>>1)); // division by 2 == >faster way to do by >> 1

            int term = ( arr[i+k] < med ) ? inner_case : -arr[i];
            
            sum += term; 
        }
    }


    end = chrono::high_resolution_clock::now();
    duration = end - start;

    musrtt += duration;




    // if we dotn print the sum it will simply assume sum variabel as dead code and will ignore the operations creating mis infprmation in our  benchmark whih caims for testing only the branching perfrmance.
    cout<<" TEST : "<<t<<"\n";
    t++;

      //THIS WAS FOR WHAT IS THE ISSUES =====> HOW DO WE RESOLVE THIS NOW

    // wherever possible we try to apply maths and avoid explicit branching as compiler is pretty good at doing math then conditioning 

    // on this same unsorted loop we will run the same thign but this time with proper math and less conditioning

   }

    cout<<" OVERALL AVG. SORTED ITERATION TIME : " << (srtt.count()/100)<<"\n";
    cout<<" OVERALL AVG. UNSORTED ITERATION TIME : " <<( usrtt.count()/100)<<"\n";

    // THE BELOW TWO VALUES SHOULD BE NERAL equal as we try to remove the complete branching using some maths so that there is no branchign and no branching => no misprediction


    cout<<" OVERALL AVG. MATHEMATICAL SORTED ITERATION TIME : " << (msrtt.count()/10)<<"\n";
    cout<<" OVERALL AVG. MATHEMATICAL UNSORTED ITERATION TIME : " <<( musrtt.count()/10)<<"\n";




  

    /* =========================== INITIAL BENCHMARKS AND LEARNINGS ========================
    
     TEST : 1
 TEST : 2
 TEST : 3
 TEST : 4
 TEST : 5
 TEST : 6
 TEST : 7
 TEST : 8
 TEST : 9
 TEST : 10
     OVERALL AVG. SORTED ITERATION TIME : 5.77004e+006
    OVERALL AVG. UNSORTED ITERATION TIME : 1.66866e+007
    OVERALL AVG. MATHEMATICAL SORTED ITERATION TIME : 598510
    OVERALL AVG. MATHEMATICAL UNSORTED ITERATION TIME : 1.94854e+006
    

    while the first 2 makes sense the last 2 results  should have been same but they are not ?? and that s because we are runnig by 

    g++ -o branch branch.cpp

    // this is the dumb compiler and does no optimization => ideally the ternary operator if ti is simpel is converted to CMOV operation which oes not uses branchign insteadd realies onmath and is fast nowwhen we compile with optimization flag O3 ( super optimized compiler we will get same timimings for both cases )

    g++ -O1 -o branch branch.cpp
    ./branch.exe

     TEST : 1
 TEST : 2
 TEST : 3
 TEST : 4
 TEST : 5
 TEST : 6
 TEST : 7
 TEST : 8
 TEST : 9
 TEST : 10
 OVERALL AVG. SORTED ITERATION TIME : 479090
 OVERALL AVG. UNSORTED ITERATION TIME : 449340
 OVERALL AVG. MATHEMATICAL SORTED ITERATION TIME : 300900
 OVERALL AVG. MATHEMATICAL UNSORTED ITERATION TIME : 300300

 now here as we can see the optimization treated the ternary not like a jump but nullified jump by using CMOV  to treat this as  mathematical comparison rather then a jump, interestingly it also clever enough to identify our branched code's pattern and also optimised it as mathemtical form and thus all the 4 results are nerla same with the first 2 being a bit messy due to more instruction and the compiler needed to interepret and process them to judge them in mathetical form


 



    
    */
}

