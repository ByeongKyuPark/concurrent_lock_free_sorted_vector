#include "LockFreeMemoryBank.h"

LockFreeMemoryBank::~LockFreeMemoryBank() {
    while (Node* oldHead = mHead.load()) {
        mHead.store(oldHead->m_pNext);
        delete oldHead->m_pData;
        delete oldHead;
        oldHead = nullptr;
    }
}

void LockFreeMemoryBank::Store(std::vector<int>* vec) {
    Node* newNode = new Node(vec);
    newNode->m_pNext = mHead.load(std::memory_order_relaxed);
    while (!mHead.compare_exchange_weak(newNode->m_pNext, newNode,
        std::memory_order_release,
        std::memory_order_relaxed));
}

std::vector<int>* LockFreeMemoryBank::Get() {
    Node* oldHead = mHead.load(std::memory_order_relaxed);
    while (oldHead && !mHead.compare_exchange_weak(oldHead, oldHead->m_pNext,
        std::memory_order_release,
        std::memory_order_relaxed));
    if (oldHead) {
        std::vector<int>* res = oldHead->m_pData;
        delete oldHead;
        return res;
    }
    else {
        return new std::vector<int>();
    }
}
