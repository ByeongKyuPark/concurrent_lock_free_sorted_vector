#pragma once

#include <vector>
#include <mutex>          // mutex for synchronization
#include <deque>          // double-ended queue
#include <chrono>         // time utilities

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