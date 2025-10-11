/*
    lfqueue ()
    void push (const obj &elem)
    obj pop ()
    ~lfqueue ()
*/

template <typename obj>
lfqueue<obj>::lfqueue () {
    Node* dummy = new Node (obj{});
    head.ptr.store (dummy, std::memory_order_relaxed);
    head.count.store (0, std::memory_order_relaxed);
    tail.ptr.store (dummy, std::memory_order_relaxed);
    tail.count.store (0, std::memory_order_relaxed);
}

template <typename obj>
void lfqueue<obj>::push (const obj &elem) {
    Node* newNode = get_from_pool (elem);

    while (true) {
        Node* last = tail.ptr.load (std::memory_order_acquire);
        Node* next = last->next.load (std::memory_order_acquire);

        if (last != tail.ptr.load (std::memory_order_acquire)) continue;

        if (next == nullptr) {
            if (last->next.compare_exchange_weak (next, newNode,
                std::memory_order_release,
                std::memory_order_relaxed)) {

                tail.ptr.compare_exchange_strong (last, newNode,
                    std::memory_order_release,
                    std::memory_order_relaxed
                );
                tail.count.fetch_add (1, std::memory_order_relaxed);

                return;
            }
        }
        else {
            tail.ptr.compare_exchange_weak (last, next,
                std::memory_order_release,
                std::memory_order_relaxed
            );
            tail.count.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

template <typename obj>
obj lfqueue<obj>::pop () {
    while (true) {
        Node* first = head.ptr.load (std::memory_order_acquire);
        Node* next = first->next.load (std::memory_order_acquire);
        Node* last = tail.ptr.load (std::memory_order_acquire);

        if (first != head.ptr.load (std::memory_order_relaxed)) 
            continue;

        if (first == last) {
            if (next == nullptr)
                throw std::runtime_error ("Queue is empty...");

            tail.ptr.compare_exchange_strong (last, next,
                std::memory_order_release,
                std::memory_order_relaxed
            );
        }

        else {
            if (next == nullptr)
                continue;

            obj value = next->val;

            if (head.ptr.compare_exchange_weak (first, next,
                std::memory_order_release,
                std::memory_order_relaxed)) {
                
                head.count.fetch_add (1, std::memory_order_relaxed);
                return_to_pool (first);
                return value;
            }
        }
    }
}

template <typename obj>
lfqueue<obj>::~lfqueue () {
    Node* current = head.ptr.load(std::memory_order_acquire);

    while (current) {

    }
}


template <typename obj>
typename lfqueue<obj>::Node* lfqueue<obj>::get_from_pool (const obj& v) {
    Node* node = node_pool.load (std::memory_order_acquire);
    while (node) {
        Node* next = node->next.load (std::memory_order_relaxed);

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
