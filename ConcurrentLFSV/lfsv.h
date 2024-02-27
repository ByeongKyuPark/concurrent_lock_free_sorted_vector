#pragma once
/*
    This implementation provides a lock-free sorted vector (LFSV) optimized for high-concurrency environments. It uses two main components for efficient memory management:
    1. MemoryBank: A pool for recycling vector instances to minimize dynamic allocation overhead.
    2. GarbageRemover: Delays the deletion of vectors to ensure safe concurrent access, preventing use-after-free errors.

    The LFSV allows multiple threads to safely update and access a dynamic array without traditional locking mechanisms, leveraging atomic operations for consistency and minimizing performance bottlenecks associated with memory management in concurrent applications.
 */
#include "LockFreeGarbageRemover.h" 
#include <vector>
#include <atomic>
#include <chrono>
#include <vector>
#include <atomic>
#include <memory> // std::shared_ptr
#include "ThreadSafeQueue.h"
#include "LockFreeGarbageRemover.h"

extern GarbageRemover globalGarbageRemover; 

class LFSV {
private:
    std::atomic<std::vector<int>*> pdata;

public:
    LFSV() : pdata(new std::vector<int>()) {}
    ~LFSV();

    void Insert(int const& v);

    int operator[](int pos) {
        std::vector<int>* snapshot = pdata.load();
        return (*snapshot)[pos];
    }
};

