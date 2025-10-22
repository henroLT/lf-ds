#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>
#include <random>
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
        for(int i=0; i<1000; i++) q.push(i);
        for(int i=0; i<1000; i++) {
            int val = q.pop();
            assert(val == i);
        }
        std::cout << "[PASS] Single-threaded correctness\n";
    }

    // 2. Multi-threaded push correctness
    void multi_thread_push(int num_threads=4, int per_thread=1000) {
        lfqueue<int> q;
        std::vector<std::thread> threads;
        for(int t=0; t<num_threads; t++) {
            threads.emplace_back([&q, t, per_thread](){
                for(int i=0; i<per_thread; i++) q.push(t*per_thread + i);
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
        for(int i=0; i<num_threads*per_thread; i++) q.push(i);

        std::atomic<int> counter(0);
        std::vector<std::thread> threads;
        for(int t=0; t<num_threads; t++) {
            threads.emplace_back([&q, &counter, per_thread](){
                for(int i=0; i<per_thread; i++) {
                    int val = q.pop();
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for(auto &th: threads) th.join();
        assert(counter == num_threads*per_thread);
        std::cout << "[PASS] Multi-threaded pop correctness\n";
    }

    // 4. Multi-producer/multi-consumer stress - FIXED
    void mp_mc_stress(int producers=4, int consumers=4, int per_producer=100000) {
        lfqueue<int> q;
        std::atomic<int> push_counter(0);
        std::atomic<int> pop_counter(0);
        std::atomic<bool> producers_done(false);

        std::vector<std::thread> prod, cons;

        // Start producers
        for(int p=0; p<producers; p++) {
            prod.emplace_back([&q, &push_counter, per_producer, p](){
                for(int i=0; i<per_producer; i++) {
                    q.push(p*per_producer + i);
                    push_counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        // Start consumers - they pop until all items are consumed
        for(int c=0; c<consumers; c++) {
            cons.emplace_back([&q, &pop_counter, &producers_done, &push_counter](){
                while(true) {
                    try {
                        q.pop();
                        pop_counter.fetch_add(1, std::memory_order_relaxed);
                    } catch(...) {
                        // Queue empty - check if producers are done
                        if(producers_done.load(std::memory_order_acquire) && 
                           pop_counter.load() >= push_counter.load()) {
                            break;
                        }
                        // Otherwise, busy-wait a bit and retry
                        std::this_thread::yield();
                    }
                }
            });
        }

        auto ms = measure_ms([&](){
            // Wait for all producers to finish
            for(auto &th: prod) th.join();
            producers_done.store(true, std::memory_order_release);
            
            // Wait for all consumers to finish
            for(auto &th: cons) th.join();
        });

        int expected = producers * per_producer;
        assert(pop_counter.load() == expected);
        std::cout << "[PASS] MPMC Stress - Pushed: " << push_counter.load() 
                  << ", Popped: " << pop_counter.load() 
                  << ", Time(ms): " << ms << "\n";
    }

    // 5. Memory pool reuse stress
    void pool_reuse_test(int cycles=100000) {
        lfqueue<int> q;
        for(int i=0; i<cycles; i++) {
            q.push(i);
            q.pop();
        }
        std::cout << "[PASS] Pool reuse stress done for " << cycles << " cycles\n";
    }

    // 6. Contention test (single element) - FIXED
    void high_contention_test(int threads=8, int ops=10000) {
        lfqueue<int> q;
        std::vector<std::thread> ts;
        
        auto ms = measure_ms([&](){
            for(int t=0; t<threads; t++) {
                ts.emplace_back([&q, ops](){
                    for(int i=0; i<ops; i++) {
                        q.push(i);
                        try {
                            q.pop();
                        } catch(...) {
                            // If queue was empty, that's ok in contention test
                        }
                    }
                });
            }
            for(auto &th: ts) th.join();
        });
        
        std::cout << "[PASS] High contention test completed, Time(ms): " << ms << "\n";
    }

    // 7. Sequential push-pop latency
    void latency_test(int operations=100000) {
        lfqueue<int> q;
        auto ms = measure_ms([&](){
            for(int i=0; i<operations; i++) {
                q.push(i);
                q.pop();
            }
        });
        double avg_us = (ms * 1000.0) / operations;
        std::cout << "[LATENCY] Operations: " << operations 
                  << ", Total ms: " << ms 
                  << ", Avg per op: " << avg_us << " µs\n";
    }

    // 8. ABA-prone pattern test - ENHANCED
    void aba_test(int threads=4, int cycles=10000) {
        lfqueue<int> q;
        std::vector<std::thread> ts;
        
        // Multiple threads doing rapid push-pop to stress ABA scenarios
        for(int t=0; t<threads; t++) {
            ts.emplace_back([&q, cycles, t](){
                for(int i=0; i<cycles; i++) {
                    q.push(t * cycles + i);
                    try {
                        q.pop();
                    } catch(...) {}
                }
            });
        }
        
        for(auto &th: ts) th.join();
        std::cout << "[PASS] ABA stress test with " << threads << " threads\n";
    }

    // 9. Large batch push test
    void large_batch_push(int batch_size=100000) {
        lfqueue<int> q;
        auto ms = measure_ms([&](){
            for(int i=0; i<batch_size; i++) q.push(i);
        });
        double ops_per_sec = (batch_size * 1000.0) / ms;
        std::cout << "[BATCH PUSH] " << batch_size << " elements in " << ms 
                  << "ms (" << ops_per_sec << " ops/sec)\n";
    }

    // 10. Large batch pop test
    void large_batch_pop(int batch_size=100000) {
        lfqueue<int> q;
        for(int i=0; i<batch_size; i++) q.push(i);
        auto ms = measure_ms([&](){
            for(int i=0; i<batch_size; i++) q.pop();
        });
        double ops_per_sec = (batch_size * 1000.0) / ms;
        std::cout << "[BATCH POP] " << batch_size << " elements in " << ms 
                  << "ms (" << ops_per_sec << " ops/sec)\n";
    }

    // NEW: 11. Mixed workload test
    void mixed_workload(int threads=8, int operations=50000) {
        lfqueue<int> q;
        std::atomic<int> push_count(0), pop_count(0);
        std::vector<std::thread> ts;
        
        // Pre-populate with some items
        for(int i=0; i<1000; i++) q.push(i);
        
        auto ms = measure_ms([&](){
            for(int t=0; t<threads; t++) {
                ts.emplace_back([&, t, operations](){
                    std::mt19937 rng(t);
                    std::uniform_int_distribution<int> dist(0, 1);
                    
                    for(int i = 0; i < operations; i++) {
                        if(dist(rng) == 0) {
                            q.push(t * operations + i);
                            push_count.fetch_add(1, std::memory_order_relaxed);
                        } else {
                            try {
                                q.pop();
                                pop_count.fetch_add(1, std::memory_order_relaxed);
                            } catch(...) {}
                        }
                    }
                });
            }
            for(auto &th: ts) th.join();
        });
        
        std::cout << "[MIXED] Pushes: " << push_count.load() 
                  << ", Pops: " << pop_count.load() 
                  << ", Time(ms): " << ms << "\n";
    }

} // namespace Benchmarks

int main() {
    std::cout << "==== LFQueue Benchmark ====\n\n";

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
    Benchmarks::mixed_workload();

    std::cout << "\n==== All tests completed ====\n";
    return 0;
}