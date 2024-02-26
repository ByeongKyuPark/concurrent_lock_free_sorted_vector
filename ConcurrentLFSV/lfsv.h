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
#include <mutex>          // mutex for synchronization
#include <deque>          // double-ended queue
#include <chrono>         // time utilities
#include "LockFreeMemoryBank.h"

class GarbageRemover {
    std::deque<std::pair<std::vector<int>*, std::chrono::time_point<std::chrono::system_clock>>> mToBeDeleted; // pairs of vectors and their deletion timestamps
    std::mutex mMutex; // protects access to to_be_deleted
    std::atomic<bool> mStop{ false }; // signals the background thread to stop
    std::thread mWorker; // background thread for deleting vectors

    void WatchingThread();

public:
    GarbageRemover() { mWorker = std::thread(&GarbageRemover::WatchingThread, this); } // start the background thread

    ~GarbageRemover();

    void Add(std::vector<int>* p) { // adds a vector to be deleted later
        std::lock_guard<std::mutex> lock(mMutex);
        mToBeDeleted.push_back(std::make_pair(p, std::chrono::system_clock::now()));
    }
    void Stop();
};

class LFSV {
    std::atomic<std::vector<int>*> pdata; // current vector, atomically updated
    GarbageRemover gr; // handles safe deletion of old vectors
    LockFreeMemoryBank mb; // pool of pre-allocated vectors

public:
    LFSV() : pdata(new std::vector<int>()), mb{} {} // initializes pdata with a new vector

    ~LFSV() { // ensures the last data vector is also safely deleted
        auto* lastData = pdata.load();
        if (lastData) gr.Add(lastData);
        gr.Stop(); // make sure to stop the GarbageRemover
    }

    void Insert(int const& v) { // inserts a value into the vector in a thread-safe manner
        std::vector<int>* pdata_old = nullptr, * pdata_new = nullptr;
        do {
            pdata_old = pdata.load(); // load the current vector
            pdata_new = mb.Get(); // get a pre-allocated vector from the MemoryBank
            *pdata_new = *pdata_old; // copy current data
            pdata_new->push_back(v); // insert the new value
            if (pdata.compare_exchange_weak(pdata_old, pdata_new)) {
                gr.Add(pdata_old); // old data goes to GarbageRemover
            }
            else {
                mb.Store(pdata_new); // if CAS fails, return the vector to the MemoryBank
            }
        } while (pdata_new != pdata.load()); // loop until successful
    }

    int operator[](int pos) { // accesses an element of the vector, potentially unsafe in concurrent scenarios
        std::vector<int>* snapshot = pdata.load(); // get the current vector
        return (*snapshot)[pos]; // return the requested element
    }
};
