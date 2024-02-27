#pragma once
/*
    This 'GarbageRemover' class provides an automated, thread-safe mechanism for delayed memory management,
    particularly designed to handle the unique challenges of the Lock-Free Sorted Vector (LFSV) in high-concurrency environments.
    It utilizes a 'ThreadSafeQueue' to schedule objects for deletion at a future time, ensuring that memory is not freed while it might still be accessed by other threads, thus preventing use-after-free errors.
    This mechanism is essential for the LFSV, allowing it to update and access data across multiple threads without traditional locking, minimizing performance bottlenecks.
    By deferring deletion, 'GarbageRemover' enables the LFSV to maintain high performance and consistency in concurrent applications, making it an integral component of the system's memory management strategy.
*/

#include "ThreadsafeQueue.h" // Include the threadsafe_queue implementation
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

class GarbageRemover {
    ThreadSafeQueue<std::pair<std::vector<int>*, std::chrono::time_point<std::chrono::system_clock>>> mToBeDeleted;
    std::atomic<bool> mStop{ false };
    std::thread mWorker;

    void WatchingThread() {
        const std::chrono::milliseconds SLEEP_TIME{ 50 };
        while (!mStop) {
            auto now = std::chrono::system_clock::now();
            std::shared_ptr<std::pair<std::vector<int>*, std::chrono::time_point<std::chrono::system_clock>>> item;
            while ((item = mToBeDeleted.TryPop())) {
                if (item->second <= now) {
                    delete item->first; // safe delete
                    std::this_thread::sleep_for(SLEEP_TIME); // sleep to not use CPU
                }
                else {
                    // if the item is not ready to be deleted, push it back for later processing
                    // (this may need a strategy to prevent immediate reprocessing, such as a temporary delay or a separate storage for future re-check)
                    mToBeDeleted.Push(std::make_pair(item->first, item->second));
                    std::this_thread::sleep_for(SLEEP_TIME); // sleep to prevent tight loop on immediate re-check
                    break; // break to check the mStop flag
                }
            }
        }
    }

public:
    GarbageRemover() {
        mWorker = std::thread(&GarbageRemover::WatchingThread, this);
    }

    ~GarbageRemover() {
        mStop = true;
        if (mWorker.joinable()) {
            mWorker.join();
        }
    }

    void ScheduleForDeletion(std::vector<int>* ptr, std::chrono::time_point<std::chrono::system_clock> deleteAfter) {
        mToBeDeleted.Push(std::make_pair(ptr, deleteAfter));
    }
};
