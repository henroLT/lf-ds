#pragma once

#include "AtomicPointerWrapper"
#include <atomic>
#include <stdexcept>
#include <cstddef>


template <typename obj>
class lfqueue {


    struct Node {
        obj val;
        std::atomic<Node*> next;

        Node()                      : val(obj{}), next(nullptr) {}
        Node(const obj& v)          : val(v), next(nullptr) {}
        Node(const obj& v, Node* n) : val(v), next(n) {}
        ~Node()                     = default;
    };

    struct PointerWrapper {
        Node* node_ptr;
        uint64_t cnt;
    };


    private:
        alignas(128) std::atomic<PointerWrapper> head;
        alignas(128) std::atomic<PointerWrapper> tail;
        std::atomic<Node*> node_pool;

        Node* get_from_pool(const obj& val);
        void return_to_pool(Node* node);

    public:
        lfqueue();
        void push(const obj &elem);
        obj pop();
        ~lfqueue();

        lfqueue(const lfqueue&) = delete;
        lfqueue& operator=(const lfqueue&) = delete;

};

#include "impl.hpp"