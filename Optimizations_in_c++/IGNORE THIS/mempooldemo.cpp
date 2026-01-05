#include<iostream>
#include<vector>
#include<array>
#include<utility>
#include<new>


#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)

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

    public :
    explicit MemPool(size_t element_count){
        // constructor 
        // explicit used to disable implicit conversion of arguements, preventing undefined behavior when argument types dont match and simply resulting in an error
        store(element_count,{T(),true}); // initialize store with the expected number of object 
        
        // Assert here t check that the first member of ObjectBool block is the T object and not the flag 
        // benefits mentioned in diary on page 1
        ASSERT(reinterpret_cast<*ObjectBlock> &(store[0].object) == &(store[0]), " ERROR : First member of ObjectBlock must be the template object \n");
        // what does reinterpret_cast do ==> changes the type of pointer from one to other 
        // syntactical use involves

        // reinterpret_cast<*target_pointer_type> &(original pointer)
        // ==> this keeps the address pointer by hexadecimal &(original pointer) as same, but just changes it's type from whetever it was to tyeh target_pointer_type 
        // we used it here to change the type of POINTER_TO_USER_OBJECT to POINTER_TO_OBJECT_BLOCK 

        // what this asert does ?? => this makes sure that the address of first stored user Object and the first ObjectBlock is same;
        
        // delete unnecessary other constructors to prevent any undefined conversion and behaviors, also this enforces that only a single manager(constructor) manages the initialization
        
    }

    // DELETE EXTRA CONSTRUCTORS 
        MemPool() = delete;
        MemPool(const MemPool&) = delete;
        MemPool(const MemPool&&) = delete;
        MemPool& operator=(const MemPool&) = delete;
        MemPool& operator=(const MemPool&&) = delete;

    template<typename... Args> 
    // T* means the response of this function will be a variable which points to on entity of Type T whihc is what we exactly want to do here.
    T* allocate(Args&&... args) noexcept {            // ALLOCATOR with universal references    
        auto obj_ptr = &(store[next_free_index]);

        ASSERT(!(obj_ptr->is_occupied), " ERROR : Expected free space at :"+std::to_string(next_free_index)+" index in MemPool");
        // how asserts work is ==> when the condition mentioned inside is true then it will move on else it will give error message 

        // we want to ensurebefore movign on that the space we are allocating is non allocatd i.e. is_allocated is false.



        T* ptr = &(store[next_free_index].object);
        // a ptr, pointing to a data of Type T;

        // ptr = new T(args...);    ==> CRITICAL BUG ==> this is standard allocation whihc means it will ask the OS to fetch storage from heap and allocate here 

        // to give memory from our memory pool we use the placement new, what it does is it will take teh location you are pointing to and construct the Object exactly at that location in memory.

        // i.e why in thsi step we create T at the exact memory location inside our mempool not asking the OS for memory from heap
        new(ptr) T(std::forward<Args>(args)...);  
        //std::forward<Args>(args)... ==> perfect forwarding ==> it transfers the arguements received as it is without copying

        obj_ptr->is_occupied = true; // this is occupied now

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

        const auto index = reinterpret_cast<ObjectBlock *>(object_pointer) - &(store[0]);

        ASSERT(index >= 0 && static_cast<size_t>(index) < store.size(), " Object with address : ",object_pointer," does not lie in MemPool ");

        ASSERT(store[index].is_occupied, " Expected the index : ",index, " to be occupied, but it is free.")

        store[index].is_occupied = false;

        // next_free_index = index; // this may be good, may be bad 
        // why good ==> sets the free index to a free location so that allocation after deallocation is fastyer as it bypasses scan 
        // why bad ?? ==> updating the free_index_pointer is the responsibility of updateNextFreeIndex if we do it here then in future when in some cvases deallocate and 
        // upDate next pointer gets called at same point of time then a race condition may occuur and since we are not using locks here data corruption may take place 
        // violating the single responsibility principle 


    }

    // an optimization to kaintain the list of free indices is to maintain a free list stack/ queue whihc will allow O(1) free space searches instead of linear scans we will test it in some time

};



// TESTING now 

class my_data {

    int avg;
    std::array<int,10> arr;

    public : 
    my_data() {
        avg = 9;
        arr.fill(9);
    }
};

int main() {

    MemPool<my_data> pool_one(size_t(10000)); // say we expect a object pool of 10,000 elements 

    for(int i = 0 ; i < 10000; i++) {

        if(UNLIKELY((i%10 )== 0)) { // happens 1/10th time 
            std::cout<<" Deallocation  time \n";
        } else {
            std::cout<<"Rest time \n";
        }
    }

    return 0;
}