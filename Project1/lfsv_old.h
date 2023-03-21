#include <iostream>       // std::cout
#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <vector>         // std::vector
#include <deque>          // std::deque
#include <mutex>          // std::mutex

struct Pair {
    std::vector<int>* pointer;
    long              ref_count;
};//__attribute__((aligned(16),packed));

class MemoryBank {
    std::deque< std::vector<int>* > slots;
    std::mutex m;
public:
    MemoryBank() : slots(6000) {//6000
        for (int i = 0; i < 6000; ++i) {//6000
            slots[i] = reinterpret_cast<std::vector<int>*>(new char[sizeof(std::vector<int>)]);
        }
    }
    std::vector<int>* Get() {
        std::lock_guard<std::mutex> lock(m);
        std::vector<int>* p = slots[0];
        slots.pop_front();
        return p;
    }
    void Store(std::vector<int>* p) {
        std::lock_guard<std::mutex> lock(m);
        slots.push_back(p);
    }
    ~MemoryBank() {
        for (auto& el : slots) {
            delete[] reinterpret_cast<char*>(el);
        }
    }

};

class GarbageRemover {
    std::deque<
        std::pair<
        Pair,
        std::chrono::time_point<std::chrono::system_clock>
        >
    > to_be_deleted;
    std::mutex m;
    std::atomic<bool> stop;
    std::thread worker;
    MemoryBank& mb;
    friend class LFSV;
    // run this method once per application
    void WatchingThread() {
        while (!stop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            std::lock_guard<std::mutex> lock(m);
            if (!to_be_deleted.empty()) {
                //peek
                std::chrono::duration<double> how_old = std::chrono::system_clock::now() - to_be_deleted[0].second;
                if (how_old > std::chrono::duration<int, std::milli>(250)
                    && to_be_deleted[0].first.ref_count == 1) {
                    std::vector<int>* p = to_be_deleted[0].first.pointer;
                    //to_be_deleted[0].first.ref_count = 0;
                    to_be_deleted.pop_front();
                    p->~vector();
                    mb.Store(p);

                    //delete[] reinterpret_cast<char*>(p);
                    //mb.Store(p);
                }
            }
        }
        // free the rest 
        for (auto& pt : to_be_deleted) {
            delete[] reinterpret_cast<char*>(pt.first.pointer);
        }
    }
public:
    GarbageRemover(MemoryBank& mb_) : to_be_deleted(), m(), stop(false), mb{mb_} {
        worker = std::thread(&GarbageRemover::WatchingThread, this);
    }
    ~GarbageRemover() {
        worker.join();
    }
    // method may be called concurrently
    void Add(std::vector<int>* p) {
        std::lock_guard<std::mutex> lock(m);
        to_be_deleted.push_back(std::make_pair(Pair{ p ,1 }, std::chrono::system_clock::now()));
    }
    void Stop() { stop = true; }
};

class LFSV {
    MemoryBank mb;
    GarbageRemover gr;
    std::atomic< Pair > pdata;

public:
    //mb.Store(pdata_old.pointer);

    LFSV() : gr{mb}, pdata{ { new (mb.Get()) std::vector<int>,1 } } {
        //pdata = { new (mb.Get()) std::vector<int>,1 };
        //        std::cout << "Is lockfree " << pdata.is_lock_free() << std::endl;
    }

    ~LFSV() {
        pdata.load().pointer->~vector();
        mb.Store(pdata.load().pointer);
        gr.Stop();
    }

    void Insert(int const& v) {
        Pair pdata_new, pdata_old;
        pdata_new.pointer = nullptr;
        do {
            //delete pdata_new.pointer;
            if (pdata_new.pointer) {        
                //pdata_new.pointer->~vector();   
                gr.Add(pdata_old.pointer);
                //mb.Store(pdata_new.pointer);
            }

            pdata_old.pointer = pdata.load().pointer;

            pdata_old.ref_count = pdata.load().ref_count;
            pdata_old.ref_count = 1;
            pdata_new.pointer = new (mb.Get()) std::vector<int>(*pdata_old.pointer);
            pdata_new.ref_count = 1;

            // working on a local copy
            std::vector<int>::iterator b = pdata_new.pointer->begin();
            std::vector<int>::iterator e = pdata_new.pointer->end();
            if (b == e || v >= pdata_new.pointer->back()) { pdata_new.pointer->push_back(v); } //first in empty or last element
            else {
                for (; b != e; ++b) {
                    if (*b >= v) {
                        pdata_new.pointer->insert(b, v);
                        break;
                    }
                }
            }

        } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

        //pdata_old.pointer->~vector();
        gr.Add(pdata_old.pointer);
    }

    int operator[] (int pos) { // not a const method anymore
        Pair pdata_new, pdata_old;
        //        std::cout << "Read from " << pdata_new.pointer;
        do { // before read - increment counter, use CAS
            pdata_old = pdata.load();
            pdata_new = pdata_old;
            ++pdata_new.ref_count;
        } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

        int ret_val = (*pdata_new.pointer)[pos];

        do { // before return - decrement counter, use CAS
            pdata_old = pdata.load();
            pdata_new = pdata_old;
            --pdata_new.ref_count;
        } while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));

        return ret_val;
    }
};
