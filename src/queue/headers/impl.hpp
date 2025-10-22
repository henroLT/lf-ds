#pragma once

#include "AtomicPointerWrapper.hpp"
#include <stdexcept>

/*
    lfqueue ()
    void push (const obj &elem)
    obj pop ()
    ~lfqueue ()

    Node* get_from_pool (const obj& val)
    void return_to_pool (Node* node)
*/

template <typename obj>
lfqueue<obj>::lfqueue() {
    Node* dummy = new Node(obj{});
    PointerWrapper<Node> pw(dummy, 0);

    head.store(pw, std::memory_order_relaxed);
    tail.store(pw, std::memory_order_relaxed);

    node_pool.store(nullptr, std::memory_order_relaxed);
}

template <typename obj>
void lfqueue<obj>::push(const obj &elem) {
    Node* newNode = get_from_pool(elem);

    while (true) {
        PointerWrapper<Node> last = tail.load(std::memory_order_acquire);
        PointerWrapper<Node> next_pw = last.node_ptr->next.load(std::memory_order_acquire);
        Node* next = next_pw.node_ptr;

        if (next == nullptr) {
            PointerWrapper<Node> desired(newNode, next_pw.cnt + 1);

            if (last.node_ptr->next.compare_exchange_weak(
                next_pw, desired,
                std::memory_order_release,
                std::memory_order_relaxed)) {

                PointerWrapper<Node> newTail(newNode, last.cnt + 1);
                tail.compare_exchange_strong(
                    last, newTail,
                    std::memory_order_release,
                    std::memory_order_relaxed
                );

                return;
            }

        } else {
            PointerWrapper<Node> newTail(next, last.cnt + 1);
            tail.compare_exchange_weak(
                last, newTail,
                std::memory_order_release,
                std::memory_order_relaxed
            );
        }
    }
}

template <typename obj>
obj lfqueue<obj>::pop() {
    while (true) {
        PointerWrapper<Node> first = head.load(std::memory_order_acquire);
        PointerWrapper<Node> last = tail.load(std::memory_order_acquire);
        PointerWrapper<Node> next_pw = first.node_ptr->next.load(std::memory_order_acquire);
        Node* next = next_pw.node_ptr;

        if (first.node_ptr == last.node_ptr) {
            if (next == nullptr)
                throw std::runtime_error("Queue is empty...");

            PointerWrapper<Node> newTail(next, last.cnt + 1);
            tail.compare_exchange_strong(
                last, newTail,
                std::memory_order_release,
                std::memory_order_relaxed
            );
        } else {
            if (next == nullptr)
                continue;

            PointerWrapper<Node> newHead(next, first.cnt + 1);
            if (head.compare_exchange_weak(
                first, newHead,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {
                
                obj value = next->val;
                return_to_pool(first.node_ptr);

                return value;
            }
        }
    }
}

template <typename obj>
lfqueue<obj>::~lfqueue() {
    PointerWrapper<Node> cur = head.load(std::memory_order_relaxed);
    Node* node = cur.node_ptr;

    while(node) {
        PointerWrapper<Node> next_pw = node->next.load(std::memory_order_relaxed);
        Node* next = next_pw.node_ptr;
        delete node;
        node = next;
    }

    Node* pool = node_pool.load(std::memory_order_relaxed);
    while (pool) {
        PointerWrapper<Node> next_pw = pool->next.load(std::memory_order_relaxed);
        Node* next = next_pw.node_ptr;
        delete pool;
        pool = next;
    }
}

template <typename obj>
typename lfqueue<obj>::Node* lfqueue<obj>::get_from_pool(const obj& v) {
    Node* node = node_pool.load(std::memory_order_acquire);

    while (node) {
        PointerWrapper<Node> next_pw = node->next.load(std::memory_order_acquire);
        Node* next = next_pw.node_ptr;

        if (node_pool.compare_exchange_weak(
            node, next,
            std::memory_order_acquire,
            std::memory_order_acquire)) {

            new (&node->val) obj(v);
            node->next.store(PointerWrapper<Node>(nullptr, 0), std::memory_order_relaxed);

            return node;
        }

        node = node_pool.load(std::memory_order_acquire);
    }

    return new Node(v);
}

template <typename obj>
void lfqueue<obj>::return_to_pool(Node* node) {
    node->val.~obj();
    std::atomic_thread_fence(std::memory_order_release);

    Node* pool_head = node_pool.load(std::memory_order_relaxed);

    do {
        node->next.store(PointerWrapper<Node>(pool_head, 0), std::memory_order_relaxed);
    } while (!node_pool.compare_exchange_weak(
        pool_head, node,
        std::memory_order_release,
        std::memory_order_relaxed
    ));
}
