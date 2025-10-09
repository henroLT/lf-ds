#ifndef NODE_HPP
#define NODE_HPP


#include <atomic>

template <typename T>
struct node {
    std::atomic<T> val;
    std::atomic<nodePointer> next;
};

template <typename T>
struct nodePointer {
    std::atomic<node<T>*> nodeP;
    std::atomic<unsigned> count;
};

template <typename T>
node<T>* node_init(const T& value) {
    std::atomic<node<T>> new_node = new node<T>;
    new_node->val.store(value);
    new_node->next.store(NULL);
    return new_node;
}


#endif // NODE_HPP