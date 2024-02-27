#include "lfsv.h"

LFSV::~LFSV() {
    std::vector<int>* toDelete = pdata.load();
    globalGarbageRemover.ScheduleForDeletion(toDelete, std::chrono::system_clock::now()); // immediately schedule for deletion
}

void LFSV::Insert(int const& v) {
    std::vector<int>* oldData = pdata.load();
    std::vector<int>* newData = new std::vector<int>(*oldData);
    // find the correct position to insert the new value to keep the vector sorted
    auto it = std::lower_bound(newData->begin(), newData->end(), v);
    newData->insert(it, v); // insert value in the correct position

    if (pdata.compare_exchange_strong(oldData, newData)) {
        // schedule the old data for deletion
        globalGarbageRemover.ScheduleForDeletion(oldData, std::chrono::system_clock::now() + std::chrono::seconds(20)); // adjust timing as needed
    }
    else {
        // if the exchange was not successful, delete the newly allocated data
        delete newData;
    }
}
