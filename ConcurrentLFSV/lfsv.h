#pragma once
/*
    This implementation provides a lock-free sorted vector (LFSV) optimized for high-concurrency environments. 
    While initially considering the use of a Memory Bank for recycling vector instances to minimize dynamic allocation overhead, 
    this version instead leverages the GarbageRemover component for efficient memory management.

    The GarbageRemover component delays the deletion of vectors to ensure safe concurrent access and prevent use-after-free errors, 
    which has been found to be more efficient for this specific application context over the traditional Memory Bank approach.

    The LFSV allows multiple threads to safely update and access a dynamic array without traditional locking mechanisms, 
    leveraging atomic operations for consistency. 
    This design minimizes the performance bottlenecks typically associated with memory management in concurrent applications, 
    making it particularly suitable for high-performance, multi-threaded environments.
*/
#include <vector>
#include <atomic>
#include <chrono>
#include <vector>
#include <atomic>
#include <memory> // std::shared_ptr
#include "GarbageRemover.h" 

extern GarbageRemover gGarbageRemover; 

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

