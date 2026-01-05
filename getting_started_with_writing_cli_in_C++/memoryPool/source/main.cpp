#include<iostream>
#include<vector>
#include<array>
#include<chrono>
#include<queue>

#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)


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


// definition of this memory pool class, we may define pools of different different types based on objects types 
template<typename T>
class MemPool {
    // memblock struct
    private : 
    struct ObjectBlock{
        T object;
        bool is_occupied = false;
    };

    // the actual array of struct 
    std::vector<ObjectBlock> store;
    size_t next_free_index = 0; 
    std::queue<size_t> freeList;

    public :
    explicit MemPool(size_t element_count){
        // constructor 
        // explicit used to disable implicit conversion of arguements, preventing undefined behavior when argument types dont match and simply resulting in an error
        store.resize(element_count,{T(),false}); // initialize store with the expected number of object 
        
        for(int i = 1;i < element_count; i++) {
            freeList.push(i);
        }
        // Assert here t check that the first member of ObjectBool block is the T object and not the flag 
        // benefits mentioned in diary on page 1
        ASSERT(reinterpret_cast<ObjectBlock*> (&(store[0].object)) == &(store[0]), " ERROR : First member of ObjectBlock must be the template object \n");
        // what does reinterpret_cast do ==> changes the type of pointer from one to other 
        // syntactical use involves

        // reinterpret_cast<*target_pointer_type> &(original pointer)
        // ==> this keeps the address pointer by hexadecimal &(original pointer) as same, but just changes it's type from whetever it was to tyeh target_pointer_type 
        // we used it here to change the type of POINTER_TO_USER_OBJECT to POINTER_TO_OBJECT_BLOCK 

        // what this asert does ?? => this makes sure that the address of first stored user Object and the first ObjectBlock is same;
        
        // delete unnecessary other constructors to prevent any undefined conversion and behaviors, also this enforces that only a single manager(constructor) manages the initialization
        
    }

    // removing default and extra constructors 

        MemPool() = delete;
        MemPool(const MemPool&) = delete;
        MemPool(const MemPool&&) = delete;
        MemPool& operator=(const MemPool&) = delete;
        MemPool& operator=(const MemPool&&) = delete;


    template<typename... Args> 
    // T* means the response of this function will be a variable which points to on entity of Type T whihc is what we exactly want to do here.


    T* allocate(Args&&... args) noexcept {            // ALLOCATOR   ==> CREATES AND STORES IN THE MRMORY POOL takes the arguements to create the object 
        auto obj_ptr = &(store[next_free_index]);

        ASSERT(!(obj_ptr->is_occupied), " ERROR : Expected free space at :"+std::to_string(next_free_index)+" index in MemPool");
        // how asserts work is ==> when the condition mentioned inside is true then it will move on else it will give error message 

        // we want to ensurebefore movign on that the space we are allocating is non allocatd i.e. is_allocated is false.



        T* ptr = &(store[next_free_index].object);
        // a ptr, pointing to a data of Type T;

        // ptr = new T(args...);    ==> CRITICAL BUG ==> this is standard allocation whihc means it will ask the OS to fetch storage from heap and allocate here 

        // to give memory from our memory pool we use the placement new, what it does is it will take teh location you are pointing to and construct the Object exactly at that location in memory.

        // i.e why in thsi step we create T at the exact memory location inside our mempool not asking the OS for memory from heap
        ptr = new(ptr) T(std::forward<Args>(args)...);  
        //std::forward<Args>(args)... ==> perfect forwarding ==> it transfers the arguements received as it is without copying

        obj_ptr->is_occupied = true; // this is occupied now
        
        // std::cout<< "ALLOCATION :  allocated object at : "<<obj_ptr<<" and store["<<next_free_index<<"].is_occupied : "<<store[next_free_index].is_occupied<<"\n";

        updateNextFreeIndex(); // update the nextFreeIndex variable


        return ptr;
    }

    auto updateNextFreeIndex() noexcept {
        size_t cur_ind = next_free_index; // to keep track of where we are right now;

        while(store[next_free_index].is_occupied) {
            // untill the current pointer is updated 
            next_free_index++;
            if(UNLIKELY(next_free_index == store.size())) {
                // loop around and set the next_free index to 0
                next_free_index = 0;
            }

            if(UNLIKELY(next_free_index == cur_ind)) {
                // we came back where we started from ==> no free space in MemPool
               ASSERT(cur_ind != next_free_index, "ERROR : MemPool out of space. ");
            }
        }
    }




    // DEALLOCATOR
    auto deAllocate(T *object_pointer) noexcept {

        const auto index = reinterpret_cast<ObjectBlock *>(object_pointer) - &(store[0]); // this might seem like that we are just finding the difference between address location whihc would result in raw bytes difference but C++ automatically divides this by object size inorder to get the index

        ASSERT(index >= 0 && static_cast<size_t>(index) < store.size(), " Object with address : " + std::to_string(size_t(object_pointer)) + " does not lie in MemPool ");
        ASSERT(store[index].is_occupied, " Expected the index : " + std::to_string(index) + " to be occupied, but it is free.");

        store[index].is_occupied = false;

        // std::cout<<"DEALLOCATION : deallocation happened at index : "<<index<<" at memory address : "<<(&store[index])<<" and store["<<index<<"].is_occupied : "<<store[index].is_occupied<<"\n";
    }

    auto updateNextFreeIndexFreeList() noexcept {

        ASSERT(UNLIKELY(freeList.size() > 0), " MEMORY FULL ");
            

        next_free_index = freeList.front();
        freeList.pop();
    }

    auto deAllocateFreeList(T *object_pointer) noexcept {

        const auto index = reinterpret_cast<ObjectBlock *>(object_pointer) - &(store[0]); // this might seem like that we are just finding the difference between address location whihc would result in raw bytes difference but C++ automatically divides this by object size inorder to get the index

        ASSERT(index >= 0 && static_cast<size_t>(index) < store.size(), " Object with address : " + std::to_string(size_t(object_pointer)) + " does not lie in MemPool ");
        ASSERT(store[index].is_occupied, " Expected the index : " + std::to_string(index) + " to be occupied, but it is free.");

        store[index].is_occupied = false;
        freeList.push(index);

        // std::cout<<"FREELIST DEALLOCATION : deallocation happened at index : "<<index<<" at memory address : "<<(&store[index])<<" and store["<<index<<"].is_occupied : "<<store[index].is_occupied<<"\n";
    }

    template<typename... Args> 
    T* allocateFreeList(Args&&... args) noexcept {            // ALLOCATOR   ==> CREATES AND STORES IN THE MRMORY POOL takes the arguements to create the object 
        auto obj_ptr = &(store[next_free_index]);

        ASSERT(!(obj_ptr->is_occupied), " ERROR : Expected free space at :"+std::to_string(next_free_index)+" index in MemPool");
        // how asserts work is ==> when the condition mentioned inside is true then it will move on else it will give error message 

        // we want to ensurebefore movign on that the space we are allocating is non allocatd i.e. is_allocated is false.



        T* ptr = &(store[next_free_index].object);
        // a ptr, pointing to a data of Type T;

        // ptr = new T(args...);    ==> CRITICAL BUG ==> this is standard allocation whihc means it will ask the OS to fetch storage from heap and allocate here 

        // to give memory from our memory pool we use the placement new, what it does is it will take teh location you are pointing to and construct the Object exactly at that location in memory.

        // i.e why in thsi step we create T at the exact memory location inside our mempool not asking the OS for memory from heap
        ptr = new(ptr) T(std::forward<Args>(args)...);  
        //std::forward<Args>(args)... ==> perfect forwarding ==> it transfers the arguements received as it is without copying

        obj_ptr->is_occupied = true; // this is occupied now
        
        // std::cout<< "ALLOCATION :  allocated object at : "<<obj_ptr<<" and store["<<next_free_index<<"].is_occupied : "<<store[next_free_index].is_occupied<<"\n";

        updateNextFreeIndexFreeList(); // update the nextFreeIndex variable


        return ptr;
    }






};



// TESTING now 

class my_data {

    int avg;
    std::array<int,10> arr;

    public :
    my_data(int x = 0) {
        avg = x;
        arr.fill(x);
    }
};



// NOTE 1 : Since this allocation will create a pool and would take time in initialization we would need to run this before the hot path starts
// NOTE 2 : The current algorithm used in our memory pool uses a scannig approach for finding free blocks, but we will use a free list stack
// based approach to bypass scan and achieved allocation in constant time complexity

int main() {

    // ================================ NORMAL TESTING  =================================
    MemPool<my_data> pool_one(100000); // say we expect a object pool of 10,000 elements 

    // pointer to last allocated block 
    my_data *last_pointer = nullptr;

    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 1 ; i < 100000; i++) {

        if(UNLIKELY((i%5 )== 0)) { // happens 1/10th time 
            // de allocate the last allocated block 
            pool_one.deAllocate(last_pointer);
        } else {    
            // allocate
            last_pointer = pool_one.allocate(i);
        }
    }  

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double,std::nano> duration = end - start;

    std::cout<<" NORMAL AVG Allco/Deallco latency : "<<duration.count()/10000<<" ns \n"; // 

    MemPool<my_data> pool_two(100000);

    my_data *ptr = nullptr;
     start = std::chrono::high_resolution_clock::now();

    for(int i = 1 ; i < 100000; i++) {
        if(UNLIKELY((i%5 )== 0)) { // happens 1/10th time 
            // de allocate the last allocated block 
            pool_two.deAllocateFreeList(ptr);
        } else {
            // allocate
            ptr = pool_two.allocateFreeList(i);
        }
    }  

    end = std::chrono::high_resolution_clock::now();

    duration = end - start;

    std::cout<<" USING NEW APPROACH  Allco/Deallco latency : "<<duration.count()/10000<<" ns \n"; // 

    // ====================================================================================



    // testing normally like this will result linear scan to be faster due to best case fo scannig as the next one is free 


    // the actual benchmarks will surface when the object pool is fragmented and then we do this loop several times ===> in our main project we will use the free List approach 







    return 0;
}