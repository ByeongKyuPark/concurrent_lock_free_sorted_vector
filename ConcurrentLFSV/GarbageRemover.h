#pragma once
/*
    This 'GarbageRemover' class provides an automated, thread-safe mechanism for delayed memory management,
    particularly designed to handle the unique challenges of the Lock-Free Sorted Vector (LFSV) in high-concurrency environments.
    It utilizes a 'ThreadSafeQueue' to schedule objects for deletion at a future time, ensuring that memory is not freed while it might still be accessed by other threads, thus preventing use-after-free errors.
    This mechanism is essential for the LFSV, allowing it to update and access data across multiple threads without traditional locking, minimizing performance bottlenecks.
    By deferring deletion, 'GarbageRemover' enables the LFSV to maintain high performance and consistency in concurrent applications, making it an integral component of the system's memory management strategy.
*/
#include "ThreadsafeQueue.h"
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>

class GarbageRemover {
    ThreadSafeQueue<std::pair<std::vector<int>*, std::chrono::time_point<std::chrono::system_clock>>> mToBeDeleted;
    std::atomic<bool> mStop{ false };
    std::thread mWorker;
    std::condition_variable mCondVar;
    std::mutex mMutex;

    void WatchingThread() {
        const std::chrono::milliseconds WAIT_TIME{ 20 };
        std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);
        while (!mStop) {
            lock.lock();
            mCondVar.wait(lock, [this] {
                return mStop || !mToBeDeleted.IsEmpty();
                });

            auto now = std::chrono::system_clock::now();
            while (!mToBeDeleted.IsEmpty() && !mStop) {
                auto item = mToBeDeleted.TryPop();
                if (item && item->second+WAIT_TIME <= now) {
                    delete item->first; // safe delete
                }
                else if (item) {
                    // schedule the thread to wake up when the next item is due for deletion
                    mCondVar.wait_until(lock, item->second);
                    // after waking up, push the item back for re-checking
                    mToBeDeleted.Push(std::make_pair(item->first, item->second));
                }
            }
            lock.unlock();
        }
    }

public:
    GarbageRemover() {
        mWorker = std::thread(&GarbageRemover::WatchingThread, this);
    }

    ~GarbageRemover() {
        mStop = true;
        mCondVar.notify_one(); // ensure the watching thread wakes up to terminate
        if (mWorker.joinable()) {
            mWorker.join();
        }
    }

    void ScheduleForDeletion(std::vector<int>* ptr) {
        mToBeDeleted.Push(std::make_pair(ptr, std::chrono::system_clock::now()));
        mCondVar.notify_one(); // notify in case the watching thread is waiting and this is the next item due
    }
};
