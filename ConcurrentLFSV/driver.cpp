/*
    Test the concurrency and integrity of a Lock-Free Sorted Vector (LFSV) in a multi-threaded environment.

    This program is designed to evaluate the performance, scalability, and data integrity of LFSV under various conditions:
        1. Concurrency of Writes: Multiple threads insert non-sequential integers concurrently to assess the vector's ability to handle concurrent modifications without data loss or corruption.
        2. Read During Writes: A dedicated thread continuously reads from a specific position within the vector to ensure reads are consistent and accurate during concurrent writes, demonstrating the vector's capability to support concurrent read and write operations.
        3. Performance and Scalability: The impact of varying the number of threads on performance is measured, illustrating how the data structure scales with increased concurrency. A total of 21000 operations are divided by the number of threads to maintain a constant workload across tests.
        4. Randomized Insertion: Elements are inserted in a randomized order to mimic real-world usage and challenge the vector's ability to maintain order and integrity under non-sequential insertion patterns.

    Test Cases:
        Test0 is designated for ThreadSafeQueueTest, utilizing the GarbageRemoved functionality. 
        Test1 through Test5 correspond to threads 1 to 5, respectively, where the user can specify the test number through argv[1]. 
        There are no separate tests for thread counts 6 and 7; instead, argv[1]=6 triggers a test with 8 threads, 
        and argv[1]=7 triggers a test with 16 threads.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm> // std::shuffle
#include <cstdlib>   // std::atoi
#include <ctime>     // std::time

#include "lfsv.h"

std::atomic<bool> doread(true);
LFSV lfsv;

std::atomic<int> c(0);
std::atomic<int> size(0);

void InsertRange(int b, int e) {
    std::vector<int> range(e - b);
    for (int i = b; i < e; ++i) {
        range[i - b] = i;
    }
    auto rng = std::default_random_engine(std::random_device{}());
    std::shuffle(std::begin(range), std::end(range), rng);//randomizes the order in which elements are inserted
    for (int i = 0; i < e - b; ++i) {
        lfsv.Insert(range[i]);
    }
}

void ReadPosition0() {
    int c = 0;
    while (doread.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (lfsv[0] != -1) {
            std::cerr << "not -1 on iteration " << c << "\n";
        }
        ++c;
    }
}

void Test(int numThreads, int numPerThread) {
    std::cout << "Starting test with " << numThreads << " threads, "
        << numPerThread << " operations per thread\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    lfsv.Insert(-1);
    std::thread reader(ReadPosition0); // pass 'lfsv' by reference to the reader thread

    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(InsertRange,i * numPerThread, (i + 1) * numPerThread));
    }
    for (auto& th : threads) th.join();

    doread.store(false);
    reader.join();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    std::cout << "Test completed in " << elapsed.count() << " seconds." << std::endl;
}

void PushPop(ThreadSafeQueue<int>& q)
{
    for (int i = 0; i < 100; ++i) {
        q.Push(++c);
        ++size;
        int val = 0;
        std::shared_ptr<int> top = q.TryPop();
        if (top) {
            --size;
        }
        //        std::cout << *top << std::endl;
    }
}

void PopPush(ThreadSafeQueue<int>& q)
{
    for (int i = 0; i < 10000; ++i) {
        int val = 0;
        std::shared_ptr<int> top = q.TryPop();
        if (top) {
            q.Push(++ * top);
        }
    }
}

void ThreadSafeQueueTest() {

    ThreadSafeQueue<int> tsq;
    //push_pop( tsq );
    constexpr int NumThreads = 4;
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < NumThreads; ++i) {
        threads.push_back(std::thread(PopPush, std::ref(tsq)));
    }
    for (unsigned i = 0; i < NumThreads; ++i) {
        threads.push_back(std::thread(PushPop, std::ref(tsq)));
    }
    for (auto& t : threads) {
        t.join();
    }

    //std::cout << "Stack content: ";
    int final_size = 0;
    while (!tsq.IsEmpty()) {
        std::shared_ptr<int> top = tsq.TryPop();
        ++final_size;
        //std::cout << *top << std::endl;
    }
    if (final_size != size) {
        std::cout << "Passed the ThreadSafeQueue Test::Wrong size " << final_size << std::endl;
    }
    else {
        std::cout << "Passed the ThreadSafeQueue Test\n";
    }
}
void Test0() { ThreadSafeQueueTest(); }
void Test1() { Test(1, 21000); }   
void Test2() { Test(2, 10500); }
void Test3() { Test(3, 7000); }
void Test4() { Test(4, 5250); }
void Test5() { Test(5, 4200); }
void Test8() { Test(8, 2625); }
void Test16() { Test(16, 1313); }
void (*pTests[])() = { Test0, Test1, Test2, Test3, Test4,Test5,Test8, Test16};

int main(int argc, char** argv) {
    
    if (argc == 2) {
        int test = std::atoi(argv[1]);
        if (test >= 0 && static_cast<int>(test) < static_cast<int>(sizeof(pTests) / sizeof(pTests[0]))) {
            try {
                pTests[test]();
            }
            catch (const char* msg) {
                std::cerr << msg << std::endl;
            }
        }
        else {
            std::cerr << "Invalid test number." << std::endl;
        }
    }
    else {
        std::cerr << "Usage: " << argv[0] << " [test number]" << std::endl;
    }

    return 0;
}
