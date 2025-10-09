#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include "node.hpp"

template <typename T>
class msQueue {
    private:
        std::atomic<nodePointer> head;
        std::atomic<nodePointer> tail;

    public:
        msQueue (T val) {
            head = new nodePointer;
            head->nodeP.next = NULL;
            tail = head;
        }

        void enqueue (T val) {
            node<T>* dummy = node_init(val);
            while(true) {
                nodePointer t = tail.load();
                nodePointer n = t->nodeP.next;
                if (t == tail) {
                    if (n.next == NULL) {
                        if (std::atomic_compare_exchange_weak(&t->nodeP.next, n, <dummy, n.count +1)){
                            break;
                        }
                    }
                    std::atomic_compare_exchange_weak(&tail, t, <n, t.count + 1);
                }
            }
        }
};