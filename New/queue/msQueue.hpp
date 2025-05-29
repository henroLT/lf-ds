#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include "node.hpp"

template <typename T>
class msQueue {
    private:
        std::atomic<node<T>*> head;
        std::atomic<node<T>*> tail;

    public:
        msQueue (T val) {
            node<T>* dummy = node_init(val);
            head.store(dummy);
            tail.store(dummy);
        }

        void enqueue (T val) {
            node<T>* dummy = node_init(val);

            while(true) {
                tail
            }
        }

};