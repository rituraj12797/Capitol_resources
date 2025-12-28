#include<iostream>
#include<vector>


// definition of this memory pool class, we may define pools of different different types based on objects types 
template<typename T> class MemPool {
    // memblock struct
    private : 
    struct ObjectBlock{
        T object;
        bool is_occupied = false;
    }

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
        MemPool() = delete;
        MemPool(const MemPool&) = delete;
        MemPool(const MemPool&&) = delete;
        MemPool& operator=(const MemPool&) = delete;
        MemPool& operator=(const MemPool&&) = delete;
    }
};

