#pragma once

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
        std::atomic<Node*> ptr;
        std::atomic<uint64_t> count;
        
        enum { PAD = (64 > sizeof(std::atomic<Node*>) + sizeof(std::atomic<uint64_t>) ? 
                64 - sizeof(std::atomic<Node*>) - sizeof(std::atomic<uint64_t>) : 1) };
        char padding[PAD];
    };


    private:
        PointerWrapper head;
        PointerWrapper tail;
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