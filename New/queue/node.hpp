#ifndef NODE_HPP
#define NODE_HPP


#include <atomic>

template <typename T>
struct node {
    std::atomic<T> val;
    std::atomic<node<T>*> next;
};

template <typename T>
node<T>* node_init(const T& value) {
    atomic<node<T>*> new_node = new node<T>;
    new_node->val.store(value);
    new_node->next.store(nullptr);
    return new_node;
}


#endif // NODE_HPP