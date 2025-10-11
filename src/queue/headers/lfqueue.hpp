#pragma once

#include <atomic>
#include <stdexcept>
#include <cstddef>


template <typename obj>
class lfqueue {

    struct NodePointer;
    struct Node;
    struct PointerWrapper;

    struct Node {
        const obj val;
        std::atomic<NodePointer*> next;

        Node()                              : val(obj{}), next(nullptr) {}
        Node(const obj& v)                  : val(v), next(nullptr) {}
        Node(const obj& v, NodePointer* n)  : val(v), next(n) {}
        ~Node()                             = default;
    };

    struct NodePointer {
        std::atomic<Node*> node;
        std::atomic<unsigned> count;

        NodePointer()                       : node(nullptr), count(0) {}
        NodePointer(Node* n)                : node(n), count(0) {}
        NodePointer(Node* n, unsigned cnt)  : node(n), count(cnt) {}
        ~NodePointer()                      = default;
    };

    struct alignas(64) PointerWrapper {
        std::atomic<NodePointer*> ptr;

        enum { PAD = (64 > sizeof(std::atomic<NodePointer*>) ? 
                64 - sizeof(std::atomic<NodePointer*>) : 1) };
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