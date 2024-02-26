#include "lfsv.h"

void GarbageRemover::WatchingThread() { // background thread function that deletes vectors after a delay
    while (!mStop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        auto now = std::chrono::system_clock::now();
        std::lock_guard<std::mutex> lock(mMutex); // delete vectors that are old enough
        while (!mToBeDeleted.empty() && now - mToBeDeleted.front().second > std::chrono::milliseconds(40)) {
            delete mToBeDeleted.front().first; // delete the vector
            mToBeDeleted.pop_front(); // remove it from the queue
        }
    }
    for (auto& pt : mToBeDeleted) delete pt.first; // clean up any remaining vectors
}

GarbageRemover::~GarbageRemover() {
    mStop = true;
    if (mWorker.joinable()) mWorker.join(); // wait for the thread to finish
}

void GarbageRemover::Stop() { // stops the background thread
    mStop = true;
    mWorker.join(); // ensure the thread finishes cleanly
}
