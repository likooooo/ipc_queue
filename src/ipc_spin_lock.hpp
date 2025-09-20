#pragma once
#include <atomic>
#include <cstdint>
#include <thread>
#include <immintrin.h>

struct ipc_spin_lock 
{
    explicit ipc_spin_lock(void* shared_memory) 
    {
        lock_ptr_ = new (shared_memory) std::atomic<uint32_t>(0);
    }

    void lock() 
    {
        while (!try_lock()) 
        {
            while (lock_ptr_->load(std::memory_order_relaxed) == 1) 
            {
#if defined(__x86_64__) || defined(_M_X64)
                _mm_pause();
#elif defined(__aarch64__) || defined(__arm__)
                asm volatile("yield" ::: "memory");
#endif
            }
        }
    }
    constexpr static uint32_t expected = 0;
    void unlock() 
    {
        lock_ptr_->store(expected, std::memory_order_release);
    }

    bool try_lock() 
    {
        uint32_t exp = expected;
        return lock_ptr_->compare_exchange_strong(
            exp, 1,
            std::memory_order_acquire,
            std::memory_order_relaxed
        );
    }
private:
    std::atomic<uint32_t>* lock_ptr_;
};
