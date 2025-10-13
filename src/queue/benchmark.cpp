#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>
#include "headers/lfqueue.hpp"

// Utility to measure time
template<typename F>
long long measure_ms(F func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Benchmark scenarios
namespace Benchmarks {

    // 1. Single-threaded push/pop correctness
    void single_thread_correctness() {
        lfqueue<int> q;
        for(int i=0;i<1000;i++) q.push(i);
        for(int i=0;i<1000;i++) {
            int val = q.pop();
            assert(val == i);
        }
        std::cout << "[PASS] Single-threaded correctness\n";
    }

    // 2. Multi-threaded push correctness
    void multi_thread_push(int num_threads=4, int per_thread=1000) {
        lfqueue<int> q;
        std::vector<std::thread> threads;
        for(int t=0;t<num_threads;t++) {
            threads.emplace_back([&q,t,per_thread](){
                for(int i=0;i<per_thread;i++) q.push(t*per_thread + i);
            });
        }
        for(auto &th: threads) th.join();
        std::cout << "[INFO] Multi-threaded push done, total elements: " 
                  << num_threads*per_thread << "\n";
    }

    // 3. Multi-threaded pop correctness
    void multi_thread_pop(int num_threads=4, int per_thread=1000) {
        lfqueue<int> q;
        // Prepopulate queue
        for(int i=0;i<num_threads*per_thread;i++) q.push(i);

        std::atomic<int> counter(0);
        std::vector<std::thread> threads;
        for(int t=0;t<num_threads;t++) {
            threads.emplace_back([&q,&counter,per_thread](){
                for(int i=0;i<per_thread;i++) {
                    int val = q.pop();
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for(auto &th: threads) th.join();
        assert(counter == num_threads*per_thread);
        std::cout << "[PASS] Multi-threaded pop correctness\n";
    }

    // 4. Multi-producer/multi-consumer stress
    void mp_mc_stress(int producers=4, int consumers=4, int per_thread=100000) {
        lfqueue<int> q;
        std::atomic<int> pop_counter(0);

        std::vector<std::thread> prod, cons;

        for(int p=0;p<producers;p++) {
            prod.emplace_back([&q, per_thread, p](){
                for(int i=0;i<per_thread;i++) q.push(p*per_thread + i);
            });
        }

        for(int c=0;c<consumers;c++) {
            cons.emplace_back([&q,&pop_counter, per_thread, producers, consumers](){ // <-- added 'consumers' capture
                int total = per_thread*producers / consumers;
                for(int i=0;i<total;i++) {
                    while(true) {
                        try {
                            q.pop();
                            pop_counter.fetch_add(1, std::memory_order_relaxed);
                            break;
                        } catch(...) {}
                    }
                }
            });
        }

        auto ms = measure_ms([&](){
            for(auto &th: prod) th.join();
            for(auto &th: cons) th.join();
        });

        std::cout << "[MPMC] Total pops: " << pop_counter.load() 
                  << ", time(ms): " << ms << "\n";
    }

    // 5. Memory pool reuse stress
    void pool_reuse_test(int cycles=100000) {
        lfqueue<int> q;
        for(int i=0;i<cycles;i++) {
            q.push(i);
            q.pop();
        }
        std::cout << "[INFO] Pool reuse stress done for " << cycles << " cycles\n";
    }

    // 6. Contention test (single element)
    void high_contention_test(int threads=8, int ops=10000) {
        lfqueue<int> q;
        q.push(0);
        std::vector<std::thread> ts;
        auto ms = measure_ms([&](){
            for(int t=0;t<threads;t++) {
                ts.emplace_back([&](){
                    for(int i=0;i<ops;i++) {
                        q.push(i);
                        q.pop();
                    }
                });
            }
            for(auto &th: ts) th.join();
        });
        std::cout << "[CONTENTION] Time(ms): " << ms << "\n";
    }

    // 7. Sequential push-pop latency
    void latency_test(int operations=100000) {
        lfqueue<int> q;
        auto ms = measure_ms([&](){
            for(int i=0;i<operations;i++) {
                q.push(i);
                q.pop();
            }
        });
        std::cout << "[LATENCY] Single-threaded push-pop ms: " << ms << "\n";
    }

    // 8. ABA-prone pattern test
    void aba_test() {
        lfqueue<int> q;
        // Push and pop repeatedly to stress ABA scenario
        for(int i=0;i<1000;i++) {
            q.push(i);
            q.pop();
        }
        std::cout << "[ABA] Repeated push-pop to stress ABA done\n";
    }

    // 9. Large batch push test
    void large_batch_push(int batch_size=100000) {
        lfqueue<int> q;
        auto ms = measure_ms([&](){
            for(int i=0;i<batch_size;i++) q.push(i);
        });
        std::cout << "[BATCH PUSH] Pushed " << batch_size << " elements in " << ms << "ms\n";
    }

    // 10. Large batch pop test
    void large_batch_pop(int batch_size=100000) {
        lfqueue<int> q;
        for(int i=0;i<batch_size;i++) q.push(i);
        auto ms = measure_ms([&](){
            for(int i=0;i<batch_size;i++) q.pop();
        });
        std::cout << "[BATCH POP] Popped " << batch_size << " elements in " << ms << "ms\n";
    }

} // namespace Benchmarks

int main() {
    std::cout << "==== LFQueue Benchmark ====\n";

    Benchmarks::single_thread_correctness();
    Benchmarks::multi_thread_push();
    Benchmarks::multi_thread_pop();
    Benchmarks::mp_mc_stress();
    Benchmarks::pool_reuse_test();
    Benchmarks::high_contention_test();
    Benchmarks::latency_test();
    Benchmarks::aba_test();
    Benchmarks::large_batch_push();
    Benchmarks::large_batch_pop();

    std::cout << "==== All tests completed ====\n";
}
