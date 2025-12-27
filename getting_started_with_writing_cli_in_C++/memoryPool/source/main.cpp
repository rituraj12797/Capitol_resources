#include<iostream>
#include<vector>


// definition of this memory pool class, we may define pools of different different types based on objects types 
template<typename T> class MemPool {
    // memblock struct
    private : 
    struct ObjectBlock{
        bool is_occupied = false;
        T object;
    }

    // the actual array of struct 
    std::vector<ObjectBlock> store;
    size_t next_free_index = 0; 

    public :
    explicit MemPool(size_t element_count){
        // constructor 
        // explicit used to disable implicit conversion of arguements, preventing undefined behavior when argument types dont match and simply resulting in an error
        
    }
};

