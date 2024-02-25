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

void InsertRange(int b, int e) {
    std::vector<int> range(e - b);
    for (int i = b; i < e; ++i) {
        range[i - b] = i;
    }
    auto rng = std::default_random_engine(std::random_device{}());
    std::shuffle(std::begin(range), std::end(range), rng);
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

void Test0() { Test(1, 25600); }   
void Test1() { Test(2, 12800); }
void Test2() { Test(4, 6400); }
void Test3() { Test(8, 3200); }
void Test4() { Test(16, 1600); }

void (*pTests[])() = { Test0, Test1, Test2, Test3, Test4};

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
