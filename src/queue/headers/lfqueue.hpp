/*
    Requires 16-byte atomic CAS
    Expects no concurrent access when destructing
*/

#pragma once

#include "AtomicPointerWrapper.hpp"
#include <atomic>
#include <stdexcept>
#include <cstddef>


template <typename obj>
class lfqueue {

    struct Node {
        obj val;
        AtomicPointerWrapper<Node> next;

        Node()                      : val(obj{}), next() {}
        Node(const obj& v)          : val(v), next() {}
        ~Node()                     = default;
    };


    private:
        AtomicPointerWrapper<Node> head;
        AtomicPointerWrapper<Node> tail;
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