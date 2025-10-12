/*
    lfqueue ()
    void push (const obj &elem)
    obj pop ()
    ~lfqueue ()
*/

template <typename obj>
lfqueue<obj>::lfqueue () {
    Node* dummy = new Node (obj{});
    head.store ({dummy, 0}, std::memory_order_relaxed);
    tail.store ({dummy, 0}, std::memory_order_relaxed);
    
    node_pool.store (nullptr, std::memory_order_relaxed);
}

template <typename obj>
void lfqueue<obj>::push (const obj &elem) {
    Node* newNode = get_from_pool (elem);

    while (true) {
        PointerWrapper last = tail.load (std::memory_order_acquire);
        Node* next = last.node_ptr->next.load (std::memory_order_acquire);

        if (next == nullptr) {
            if (last.node_ptr->next.compare_exchange_weak (next, newNode,
                std::memory_order_release,
                std::memory_order_relaxed)) {

                tail.compare_exchange_strong (last, {newNode, last.cnt + 1},
                    std::memory_order_release,
                    std::memory_order_relaxed
                );

                return;
            }
        }
        else {
            tail.compare_exchange_weak (last, {next, last.cnt + 1},
                std::memory_order_release,
                std::memory_order_relaxed
            );
        }
    }
}

template <typename obj>
obj lfqueue<obj>::pop () {
    while (true) {
        PointerWrapper first = head.load (std::memory_order_acquire);
        PointerWrapper last = tail.load (std::memory_order_acquire);
        Node* next = first.node_ptr->next.load (std::memory_order_acquire);

        if (first.node_ptr == last.node_ptr) {
            if (next == nullptr)
                throw std::runtime_error ("Queue is empty...");

            tail.compare_exchange_strong (last, {next, last.cnt + 1},
                std::memory_order_release,
                std::memory_order_relaxed
            );
        }

        else {
            if (next == nullptr)
                continue;

            obj value = next->val;

            if (head.compare_exchange_weak (first, {next, first.cnt + 1},
                std::memory_order_release,
                std::memory_order_relaxed)) {
                
                return_to_pool (first.node_ptr);
                return value;
            }
        }
    }
}

template <typename obj>
lfqueue<obj>::~lfqueue () {
    
}


template <typename obj>
typename lfqueue<obj>::Node* lfqueue<obj>::get_from_pool (const obj& v) {
    Node* node = node_pool.load (std::memory_order_acquire);
    while (node) {
        Node* next = node->next.load (std::memory_order_acquire);

        if (node_pool.compare_exchange_weak (node, next,
            std::memory_order_release,
            std::memory_order_relaxed)) {

            new (&node->val) obj (v);
            node->next.store (nullptr, std::memory_order_relaxed);
            return node;
        }
    }

    return new Node (v);
}

template <typename obj>
void lfqueue<obj>::return_to_pool (Node* node) {
    Node* pool_node = node_pool.load (std::memory_order_relaxed);

    node->val.~obj ();

    do {
        node->next.store (pool_node, std::memory_order_relaxed);
    } while (!node_pool.compare_exchange_weak (pool_node, node,
        std::memory_order_release,
        std::memory_order_relaxed
    ));
}
