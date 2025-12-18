

// Optimization 1 : local variables which are defined inside function are more performant as compared to the static variables 

// WHY ?? ==> compilers tightly pack the variables inside the function closer to each other so that when a variable is needed by processor and it hts a cache miss, it fetches this variables and alogn with it it fetches cintents stored around this variable in memory as a part fo cache fetch.

// WHY this happens that processor fetches near by things ?? ==> to understand this we look into how modern procesors work, gone are days when processors uses to be sequential, nowadays most processors are vector supported and have large size registers, and due to these large size registers the processors are able to fetch values stored in nearby addresses too.

// WHY static is not cache friendly as compared to local ?? ==> the reason lies in how static variables are stored in memory, since static variables live for the whoel lifetime of the programs they occupy fixed permanent addresses in programs's data memory inside RAM,. Now each time when compiler hits a cache miss it goes to RAM takes this variable and it sorrounding values and puts that into cache, but static variables are usually scattred in the sense of addreses they are allocated. so in the fetching of one variable it may happen that it's sorrounding contains un related data which is of no relevance to this variable and hence processor will keep hitting cache miss and keep travelling to memory back and forth for dat aaccess, resulting in low performance.

// Example is shown here by executing a simpel function 1st by ue of local variables then by static variables.

#include<bits/stdc++.h>
#include<chrono>
using namespace std;

static int a = 10; // different memory location
vector<int> arr(100000,101); // in between an array to force seperation
static int b = 10; // another static var in different location

int res() {
    static int c = 10; // another static var in differend location
    return a + b + c; // 3 cache misses as all 3 vriables live at different variables
}

int res_local() {
    int aa = 10; // all three vars are local os fetching say aa prefetches bb and cc and thus performance gains are seen 
    int bb = 10;
    int cc = 10;

    return aa + bb + cc;
}

int main(){
    auto start = chrono::high_resolution_clock().now();
    int result_res = 0;
    for(int i = 0 ; i < 10000000; i++) {
        result_res += res();
    }
    auto end = chrono::high_resolution_clock().now();
    chrono::duration<double,nano>duration = end - start;
    cout<<" result of  res() : "<<result_res<<"\n";
    cout<<" time taken by res() : "<<duration.count()<<" nano sec"<<"\n";
    
    start = chrono::high_resolution_clock().now();

    int result_res_local = 0;

     for(int i = 0 ; i < 10000000; i++) {
        result_res_local += res_local();
    }

    end = chrono::high_resolution_clock().now();
    duration = end - start;
    cout<<" result of  res_local() : "<<result_res_local<<"\n";
    cout<<" time taken by res_local() : "<<duration.count()<<" nano sec"<<"\n";
}

// test with 

// g++ -o locstat locstat.cpp   ( removed the compiler optimization flag -O2 -O3  to see the actual perf. )

//