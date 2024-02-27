#include "LockFreeMemoryBank.h"
LockFreeMemoryBank::~LockFreeMemoryBank() {
    AtomicNode oldHead = mHead.load();
    while (oldHead.ptr) {
        mHead.store(AtomicNode(oldHead.ptr->m_pNext, oldHead.version + 1));
        delete oldHead.ptr->m_pData;
        delete oldHead.ptr;
        oldHead = mHead.load();
    }
}

void LockFreeMemoryBank::Store(std::vector<int>* vec) {
    Node* newNode = new Node(vec);
    AtomicNode oldHead = mHead.load(std::memory_order_relaxed);

    do {
        newNode->m_pNext = oldHead.ptr;
        // attempt to store the new Node with an incremented version number.
    } while (!mHead.compare_exchange_weak(oldHead, AtomicNode(newNode, oldHead.version + 1),
        std::memory_order_release,
        std::memory_order_relaxed));
}

std::vector<int>* LockFreeMemoryBank::Get() {
    AtomicNode oldHead = mHead.load(std::memory_order_relaxed);

    while (oldHead.ptr && !mHead.compare_exchange_weak(oldHead, AtomicNode(oldHead.ptr->m_pNext, oldHead.version + 1),
        std::memory_order_release,
        std::memory_order_relaxed));

    if (oldHead.ptr) {
        std::vector<int>* res = oldHead.ptr->m_pData;
        delete oldHead.ptr;
        return res;
    }
    else {
        return new std::vector<int>();
    }
}
