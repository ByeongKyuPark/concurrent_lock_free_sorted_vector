/*
    This implementation provides a lock-free sorted vector (LFSV) optimized for high-concurrency environments. It uses two main components for efficient memory management:
    1. MemoryBank: A pool for recycling vector instances to minimize dynamic allocation overhead.
    2. GarbageRemover: Delays the deletion of vectors to ensure safe concurrent access, preventing use-after-free errors.

    The LFSV allows multiple threads to safely update and access a dynamic array without traditional locking mechanisms, leveraging atomic operations for consistency and minimizing performance bottlenecks associated with memory management in concurrent applications.
 */

#include <iostream>    
#include <atomic>
#include <thread> 
#include <vector>
#include "LockFreeGarbageRemover.h"
#include "LockFreeMemoryBank.h"

class LFSV {
    std::atomic<std::vector<int>*> pdata; // current vector, atomically updated
    GarbageRemover gr; // handles safe deletion of old vectors
    LockFreeMemoryBank mb; // pool of pre-allocated vectors

public:
    LFSV() : pdata(new std::vector<int>()), mb{} {} // initializes pdata with a new vector

    ~LFSV();
    void Insert(int const& v);
    int operator[](int pos);
};
