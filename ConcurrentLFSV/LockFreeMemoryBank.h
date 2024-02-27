#pragma once
/**
 * LockFreeMemoryBank.h !
 * by ByeongKyu Park
 *
 * Overview:
 *   Implements a lock-free memory bank for managing a pool of pre-allocated
 *   vectors, optimized for high-concurrency environments. This class aims to
 *   minimize dynamic allocation overhead and reduce contention in scenarios
 *   where multiple threads concurrently access and recycle vector instances.
 *
 * Design:
 *   Utilizes a lock-free stack, based on atomic operations, to hold available
 *   vector instances. This approach ensures thread-safe access without the
 *   need for locking mechanisms, leveraging compare-and-swap (CAS) operations
 *   for managing the stack's head pointer.
 *
 * Integration:
 *   Designed to replace the mutex-protected MemoryBank in the LFSV (Lock-Free
 *   Sorted Vector) implementation, enhancing its performance by reducing lock
 *   contention and supporting more efficient concurrent operations.
 *
 * Usage:
 *   - Call 'Get()' to retrieve a vector from the pool. If the pool is empty, a new vector is allocated.
 *   - Use 'Store(std::vector<int>* vec)' to return a vector to the pool after use.
 *   - Ensure proper initialization and destruction within the context of the LFSV class to manage memory effectively.
 *
 * Note:
 *   This implementation focuses on concurrency and performance improvements.
 *   Further consideration may be required for memory reclamation strategies
 *   in long-running applications to avoid potential memory leaks.
 */
#include <atomic>
#include <vector>

class LockFreeMemoryBank {
private:
    struct Node {
        std::vector<int>* m_pData;
        Node* m_pNext;

        Node(std::vector<int>* data) : m_pData(data), m_pNext(nullptr) {}
    };

    struct AtomicNode {
        Node* ptr;
        unsigned long long version;

        AtomicNode(Node* pNode = nullptr, unsigned long long ver = 0) : ptr(pNode), version(ver) {}
    };

    std::atomic<AtomicNode> mHead;

public:
    LockFreeMemoryBank() : mHead(AtomicNode()) {}
    ~LockFreeMemoryBank();

    void Store(std::vector<int>* vec);
    std::vector<int>* Get();
};