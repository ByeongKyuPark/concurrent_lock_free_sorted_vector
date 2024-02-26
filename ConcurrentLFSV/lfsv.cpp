#include "lfsv.h"

LFSV::~LFSV() { // ensures the last data vector is also safely deleted
    auto* lastData = pdata.load();
    if (lastData) gr.Add(lastData);
    gr.Stop(); // make sure to stop the GarbageRemover
}

void LFSV::Insert(int const& v) { // inserts a value into the vector in a thread-safe manner
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

int LFSV::operator[](int pos) { // accesses an element of the vector, potentially unsafe in concurrent scenarios
    std::vector<int>* snapshot = pdata.load(); // get the current vector
    return (*snapshot)[pos]; // return the requested element
}