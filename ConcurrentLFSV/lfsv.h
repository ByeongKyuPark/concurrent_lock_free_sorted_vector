/*
    This implementation provides a lock-free sorted vector (LFSV) optimized for high-concurrency environments. It uses two main components for efficient memory management:
    1. MemoryBank: A pool for recycling vector instances to minimize dynamic allocation overhead.
    2. GarbageRemover: Delays the deletion of vectors to ensure safe concurrent access, preventing use-after-free errors.

    The LFSV allows multiple threads to safely update and access a dynamic array without traditional locking mechanisms, leveraging atomic operations for consistency and minimizing performance bottlenecks associated with memory management in concurrent applications.
*/

#include <iostream>       // standard I/O operations
#include <atomic>         // atomic operations for thread safety
#include <thread>         // thread handling
#include <vector>         // dynamic array
#include <mutex>          // mutex for synchronization
#include <deque>          // double-ended queue
#include <chrono>         // time utilities

class GarbageRemover {
    std::deque<std::pair<std::vector<int>*, std::chrono::time_point<std::chrono::system_clock>>> to_be_deleted; // pairs of vectors and their deletion timestamps
    std::mutex m; // protects access to to_be_deleted
    std::atomic<bool> stop{ false }; // signals the background thread to stop
    std::thread worker; // background thread for deleting vectors

    void WatchingThread() { // background thread function that deletes vectors after a delay
        while (!stop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            auto now = std::chrono::system_clock::now();
            std::lock_guard<std::mutex> lock(m); // delete vectors that are old enough
            while (!to_be_deleted.empty() && now - to_be_deleted.front().second > std::chrono::milliseconds(40)) {
                delete to_be_deleted.front().first; // delete the vector
                to_be_deleted.pop_front(); // remove it from the queue
            }
        }
        for (auto& pt : to_be_deleted) delete pt.first; // clean up any remaining vectors
    }

public:
    GarbageRemover() { worker = std::thread(&GarbageRemover::WatchingThread, this); } // start the background thread

    ~GarbageRemover() {
        stop = true;
        if (worker.joinable()) worker.join(); // wait for the thread to finish
    }

    void Add(std::vector<int>* p) { // adds a vector to be deleted later
        std::lock_guard<std::mutex> lock(m);
        to_be_deleted.push_back(std::make_pair(p, std::chrono::system_clock::now()));
    }
    void Stop() { // stops the background thread
        stop = true;
        worker.join(); // ensure the thread finishes cleanly
    }
};

class MemoryBank {
    std::deque<std::vector<int>*> mSlots; // pool of vectors
    std::mutex mMutex; // protects access to mSlots

public:
    MemoryBank(int size) { // pre-allocates a specified number of vectors
        for (int i = 0; i < size; ++i) mSlots.push_back(new std::vector<int>());
    }

    ~MemoryBank() { // deletes all vectors in the pool
        std::lock_guard<std::mutex> lock(mMutex);
        for (auto& el : mSlots) delete el;
    }

    std::vector<int>* Get() { // retrieves a vector from the pool or creates a new one if the pool is exhausted
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mSlots.empty()) {
            auto* vec = mSlots.front();
            mSlots.pop_front();
            return vec;
        }
        return new std::vector<int>();
    }

    void Store(std::vector<int>* vec) { // returns a vector to the pool for future reuse
        std::lock_guard<std::mutex> lock(mMutex);
        mSlots.push_back(vec);
    }
};

class LFSV {
    std::atomic<std::vector<int>*> pdata; // current vector, atomically updated
    GarbageRemover gr; // handles safe deletion of old vectors
    MemoryBank mb{ 5000 }; // pool of pre-allocated vectors

public:
    LFSV() : pdata(new std::vector<int>()) {} // initializes pdata with a new vector

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
            if (pdata.compare_exchange_weak(pdata_old, pdata_new)) gr.Add(pdata_old); // old data goes to GarbageRemover
            else mb.Store(pdata_new); // if CAS fails, return the vector to the MemoryBank
        } while (pdata_new != pdata.load()); // loop until successful
    }

    int operator[](int pos) { // accesses an element of the vector, potentially unsafe in concurrent scenarios
        std::vector<int>* snapshot = pdata.load(); // get the current vector
        return (*snapshot)[pos]; // return the requested element
    }
};
