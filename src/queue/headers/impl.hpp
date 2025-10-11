/*
    lfqueue()
    void push(const obj &elem)
    obj pop()
    ~lfqueue()
*/

template <typename obj>
lfqueue<obj>::lfqueue() {
    Node* dummy = new Node(obj{});
    NodePointer* dummyPtr = new NodePointer(dummy, 0);
    head.ptr.store(dummyPtr, std::memory_order_relaxed);
    tail.ptr.store(dummyPtr, std::memory_order_relaxed);
}

template <typename obj>
void lfqueue<obj>::push(const obj &elem) {
    Node* newNode = get_from_pool(elem);
    NodePointer* newPtr = new NodePointer(newNode);

    while (true) {
        NodePointer* last = tail.ptr.load(std::memory_order_acquire);
        if (!last) continue;
        Node* lastNode = last->node.load(std::memory_order_acquire);
        if (!lastNode) continue;
        NodePointer* next = lastNode->next.load(std::memory_order_acquire);

        if (next == nullptr) {
            if (lastNode->next.compare_exchange_weak(
                next, newPtr,
                std::memory_order_release,
                std::memory_order_relaxed
            )) break;
        }
        else {
            tail.ptr.compare_exchange_weak(
                last, next,
                std::memory_order_release,
                std::memory_order_relaxed
            );
        }
    }

    NodePointer* lastPtr = tail.ptr.load(std::memory_order_acquire);
    while (!tail.ptr.compare_exchange_weak(
        lastPtr, newPtr,
        std::memory_order_acq_rel,
        std::memory_order_relaxed
    )){}
}

template <typename obj>
obj lfqueue<obj>::pop() {
    while (true) {
        NodePointer* first = head.ptr.load(std::memory_order_acquire);
        if (!first) continue;
        Node* firstNode = first->node.load(std::memory_order_acquire);
        if (!firstNode) continue;
        NodePointer* next = firstNode->next.load(std::memory_order_acquire);

        NodePointer* last = tail.ptr.load(std::memory_order_acquire);
        if (!last) continue;

        if (first == last) {
            if (next == nullptr)
                throw std::runtime_error("Queue is empty...");

            tail.ptr.compare_exchange_weak(
                last, next,
                std::memory_order_release,
                std::memory_order_relaxed
            );

            continue;
        }

        if (!next) continue;
        Node* nextNode = next->node.load(std::memory_order_acquire);
        if (!nextNode) continue;
        obj value = nextNode->val;

        if (head.ptr.compare_exchange_weak(
            first, next,
            std::memory_order_acq_rel,
            std::memory_order_relaxed
        )) {
            return_to_pool(firstNode);
        }
    }
}

template <typename obj>
lfqueue<obj>::~lfqueue() {
    NodePointer* current = head.ptr.load(std::memory_order_relaxed);

    /* Delete nodes in queue */
    while (current) {
        Node* n = current->node.load(std::memory_order_relaxed);
        NodePointer* nextPtr = nullptr;

        if (n) {
            nextPtr = n->next.load(std::memory_order_relaxed);
            delete n;
        }

        delete current;
        current = nextPtr;
    }

    /* Delete nodes in pool */
}


template <typename obj>
Node* lfqueue<obj>::get_from_pool(const obj& v) {
    Node* pooled_node = node_pool.exchange(nullptr);

    if (pooled_node) {
        pooled_node->val = v;
        pooled_node->next.store(nullptr, std::memory_order_relaxed);
        return pooled_node;
    }

    return new Node(v);
}

template <typename obj>
void lfqueue<obj>::return_to_pool(Node* node) {
    Node* pool_node = node_pool.load(std::memory_order_relaxed);

    do {
        node->next.store(pool_node, std::memory_order_relaxed);
    } while (!node_pool.compare_exchange_weak(
        pool_node, node,
        std::memory_order_release;
        std::memory_order_relaxed
    ));
}
