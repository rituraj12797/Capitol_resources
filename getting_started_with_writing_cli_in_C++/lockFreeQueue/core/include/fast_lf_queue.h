#pragma once

#include <vector>
#include <atomic>
#include <cassert>
#include <new> // for hardware_destructive_interference_size

namespace internal_lib {

    template<typename T>
    class FastLFQueue final {
    private:
        struct alignas(64) Slot {
            T value;
        };

        std::vector<Slot> store_; 
        size_t capacity_mask_; // Used for bitwise wrapping

        // PADDING: Separate Read and Write lines to prevent False Sharing
        alignas(64) std::atomic<size_t> write_idx_ = {0};
        alignas(64) size_t cached_read_idx_ = 0; // SHADOW INDEX for Writer

        alignas(64) std::atomic<size_t> read_idx_ = {0};
        alignas(64) size_t cached_write_idx_ = 0; // SHADOW INDEX for Reader

    public:
        explicit FastLFQueue(size_t num_elems) {
            // Force Power of 2 size
            if (num_elems < 2) num_elems = 2;
            size_t capacity = 1;
            while (capacity < num_elems) capacity *= 2; // Next Power of 2

            store_.resize(capacity);
            capacity_mask_ = capacity - 1;
        }

        // Disable copying
        FastLFQueue(const FastLFQueue&) = delete;
        FastLFQueue& operator=(const FastLFQueue&) = delete;

        // WRITER FUNCTION
        template<typename U>
        bool push(U&& item) noexcept {
            const size_t curr_write = write_idx_.load(std::memory_order_relaxed);
            const size_t next_write = (curr_write + 1) & capacity_mask_;

            // OPTIMIZATION 3: Check SHADOW index first (Cheap!)
            if (next_write == cached_read_idx_) {
                // Shadow says full. Check the REAL atomic index (Expensive)
                cached_read_idx_ = read_idx_.load(std::memory_order_acquire);
                
                // Still full? Then we must return false (Backpressure)
                if (next_write == cached_read_idx_) {
                    return false; 
                }
            }

            // Write the data
            store_[curr_write].value = std::forward<U>(item);

            // OPTIMIZATION 2: Release ordering
            // "Publish" the write index so Reader sees the new data
            write_idx_.store(next_write, std::memory_order_release);
            
            return true;
        }

        // READER FUNCTION
        T* peek() noexcept {
            const size_t curr_read = read_idx_.load(std::memory_order_relaxed);

            // OPTIMIZATION 3: Check SHADOW index first
            if (curr_read == cached_write_idx_) {
                // Shadow says empty. Check REAL atomic index.
                cached_write_idx_ = write_idx_.load(std::memory_order_acquire);
                
                if (curr_read == cached_write_idx_) {
                    return nullptr; // Queue is truly empty
                }
            }

            // Return pointer to data
            return &store_[curr_read].value;
        }

        void pop() noexcept {
            const size_t curr_read = read_idx_.load(std::memory_order_relaxed);
            const size_t next_read = (curr_read + 1) & capacity_mask_;
            
            // "Publish" the read index so Writer sees the free slot
            read_idx_.store(next_read, std::memory_order_release);
        }
    };
}