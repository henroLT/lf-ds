#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>
#include "headers/lfqueue.hpp"

int main(int argc, char** argv) {
    using T = int;

    int numElements = 100;
    int numThreads = 2;
    if (argc > 1) numElements = std::atoi(argv[1]);
    if (argc > 2) numThreads = std::atoi(argv[2]);
    if (numElements < 0) numElements = 100;
    if (numThreads < 1) numThreads = 1;

    lfqueue<T> q;

    std::cout << "Pushing " << numElements << " elements with "
              << numThreads << " threads...\n";

    int perThread = (numElements + numThreads - 1) / numThreads;

    std::vector<std::thread> pushThreads;
    for (int t = 0; t < numThreads; ++t) {
        pushThreads.emplace_back([t, perThread, &q]() {
            int start = t * perThread;
            int end = start + perThread;
            for (int i = start; i < end; ++i) {
                q.push(static_cast<T>(i));
            }
        });
    }

    for (auto& th : pushThreads) th.join();

    std::cout << "All elements pushed.\n";


    std::vector<std::thread> popThreads;
    std::atomic<int> poppedCount{0};

    for (int t = 0; t < numThreads; ++t) {
        popThreads.emplace_back([&q, &poppedCount, numElements]() {
            while (true) {
                try {
                    T val = q.pop();
                    int count = ++poppedCount;
                    if (count % 10 == 0) std::cout << "Popped " << count << "\n";
                    if (count >= numElements) break;
                } catch (const std::runtime_error&) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& th : popThreads) th.join();

    std::cout << "All elements popped. Total popped: " << poppedCount << "\n";
    return 0;
}
