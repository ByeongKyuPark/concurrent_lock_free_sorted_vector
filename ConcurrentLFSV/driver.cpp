/*
    Test the concurrencyand integrity of a Lock - Free Sorted Vector(LFSV) in a multi - threaded environment.

    1. Concurrency of Writes 
        : Multiple threads insert non - sequential integers concurrently to evaluate the vector's ability to handle concurrent modifications.
    2. Read During Writes 
        : A dedicated thread continuously reads from a specific position within the vector to ensure that reads during concurrent writes are consistent and accurate, highlighting the structure's capacity for concurrent read and write operations without data corruption.
    3. Performance and Scalability 
        : The performance impact of varying the number of threads and operations per thread is measured, showcasing how well the data structure scales with increased concurrency.
    4. Randomized Insertion 
        : Data is inserted in a randomized order to simulate a real - world scenario, testing the vector's ability to maintain order and integrity under non-sequential writes.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm> // std::shuffle
#include <cstdlib> // std::atoi
#include <ctime> // std::time

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

void push_pop(ThreadSafeQueue<int>& q)
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

void pop_push(ThreadSafeQueue<int>& q)
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
        threads.push_back(std::thread(pop_push, std::ref(tsq)));
    }
    for (unsigned i = 0; i < NumThreads; ++i) {
        threads.push_back(std::thread(push_pop, std::ref(tsq)));
    }
    for (auto& t : threads) {
        t.join();
    }

    //std::cout << "Stack content: ";
    int final_size = 0;
    while (!tsq.empty()) {
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

void Test0() { Test(1, 25600); }   
void Test1() { Test(2, 12800); }
void Test2() { Test(4, 6400); }
void Test3() { Test(8, 3200); }
void Test4() { Test(16, 1600); }

void (*pTests[])() = { Test0, Test1, Test2, Test3, Test4};

int main(int argc, char** argv) {
    
    ThreadSafeQueueTest();

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
