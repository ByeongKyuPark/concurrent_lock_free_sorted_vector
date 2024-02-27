#pragma once
/*
    ThreadSafeQueue - Optimized for Deferred Memory Management in LFSV Systems

    Overview:
    The ThreadSafeQueue is a key concurrent data structure designed explicitly for the GarbageRemover component of Lock-Free Sorted Vector (LFSV) systems. Its primary function is to manage the deferred deletion of vector elements, ensuring thread-safe operations and the integrity of memory management in highly concurrent environments.

    Rationale for Queue over Stack:
    - Sequential Processing: The queue facilitates the sequential processing of memory deletion requests, which is essential for the GarbageRemover. This sequential approach aligns with the temporal requirements for safely deleting memory resources, where elements must be deleted in the order they become safe to remove, preventing premature deletions and use-after-free errors.
    - Deferred Deletion: Unlike a stack, which would prioritize last-in, first-out (LIFO) operations, a queue's first-in, first-out (FIFO) nature ensures that memory resources are given ample time to be released from all potential references in the system. This delay is crucial for the memory safety of concurrently accessed structures in the LFSV, where recently used elements might still be in use by other threads.
    - Memory Reclamation Strategy: The choice of a queue supports a more predictable and stable memory reclamation strategy. It aligns with the temporal patterns of garbage collection in the LFSV, where elements scheduled for deletion must be retained for a specific period before safe reclamation, optimizing memory usage and reducing fragmentation.

    Usage in LFSV GarbageRemover:
    In the context of LFSV, the ThreadSafeQueue's FIFO mechanism is instrumental for the GarbageRemover component. It queues up memory deletion tasks based on their scheduled deletion time, ensuring that no memory is freed while it might still be accessed by concurrent operations. This strategic use of a queue over a stack is a deliberate design choice to enhance cache efficiency and memory reuse, particularly in scenarios where memory access patterns are highly dynamic yet require strict management to avoid data races and memory leaks.

    By leveraging a queue, the LFSV system ensures that its components can operate with high efficiency and reliability, maintaining the integrity of its lock-free data structures and supporting the high-concurrency demands of modern software applications.
*/

#include <mutex>
#include <memory>
#include <condition_variable>
#include <chrono>
#include <thread> // std::this_thread::sleep_for

template<typename T>
class ThreadSafeQueue
{
    struct Node
    {
        std::shared_ptr<T> m_pData;
        std::unique_ptr<Node> m_pNext;
    };

    mutable std::mutex mHeadMutex; // mutable for const correctness in empty()
    mutable std::mutex mTailMutex; // mutable for const correctness in empty()
    std::condition_variable mDataCond; // for notifying waiting threads

    std::unique_ptr<Node> m_pHead;
    Node* m_pTail;

    Node* GetTail() const
    {
        std::lock_guard<std::mutex> tailLock(mTailMutex);
        return m_pTail;
    }

    std::unique_ptr<Node> PopHead()
    {
        std::lock_guard<std::mutex> headLock(mHeadMutex);
        if (m_pHead.get() == GetTail())
        {
            return nullptr;
        }
        std::unique_ptr<Node> old_head = std::move(m_pHead);
        m_pHead = std::move(old_head->m_pNext);
        return old_head;
    }

    std::unique_lock<std::mutex> WaitForData()
    {
        std::unique_lock<std::mutex> headLock(mHeadMutex);
        mDataCond.wait(headLock, [&] { return m_pHead.get() != GetTail(); });
        return std::move(headLock); // return to allow caller to hold the lock
    }

    std::unique_ptr<Node> WaitPopHead()
    {
        std::unique_lock<std::mutex> headLock(WaitForData());
        std::unique_ptr<Node> old_head = std::move(m_pHead);
        m_pHead = std::move(old_head->m_pNext);
        return old_head;
    }

public:
    ThreadSafeQueue() : m_pHead(new Node), m_pTail(m_pHead.get()) {}
    ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;

    std::shared_ptr<T> TryPop()
    {
        std::unique_ptr<Node> oldHead = PopHead();
        return oldHead ? oldHead->m_pData : std::shared_ptr<T>();
    }

    std::shared_ptr<T> WaitAndPop()
    {
        std::unique_ptr<Node> const oldHead = WaitPopHead();
        return oldHead->m_pData;
    }

    void Push(T new_value)
    {
        std::shared_ptr<T> newData(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<Node> p(new Node);
        Node* const newTail = p.get();

        {
            std::lock_guard<std::mutex> tailLock(mTailMutex);
            m_pTail->m_pData = newData;
            m_pTail->m_pNext = std::move(p);
            m_pTail = newTail;
        }
        mDataCond.notify_one();
    }

    bool IsEmpty() const
    {
        std::lock_guard<std::mutex> headLock(mHeadMutex);
        return (m_pHead.get() == GetTail());
    }
};
