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
        uint64_t last_count = tail.count.load (std::memory_order_acquire);
        Node* next = last->next.load (std::memory_order_acquire);

        if (last != tail.ptr.load (std::memory_order_acquire)) continue;

        if (next == nullptr) {
            if (last.compare_exchange_weak (next, newNode,
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

            NodePointer* newTail = new NodePointer(
                next->node.load(std::memory_order_acquire), last->count + 1
            );
            tail.ptr.compare_exchange_weak(last, newTail,
                std::memory_order_release,
                std::memory_order_relaxed
            );

            continue;
        }

        if (!next) continue;
        Node* nextNode = next->node.load(std::memory_order_acquire);
        if (!nextNode) continue;
        obj value = nextNode->val;

         NodePointer* newHead = new NodePointer(nextNode, first->count + 1);
        if (head.ptr.compare_exchange_weak(first, newHead,
            std::memory_order_acq_rel,
            std::memory_order_relaxed
        )) {
            return_to_pool(firstNode);
            return value;
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
    Node* node = node_pool.load(std::memory_order_relaxed);
    while (node) {
        Node* next = reinterpret_cast<Node*>(
            node->next.load(std::memory_order_relaxed)
        );
        delete node;
        node = next;
    }
}


template <typename obj>
Node* lfqueue<obj>::get_from_pool(const obj& v) {
    Node* node = node_pool.load(std::memory_order_relaxed);
    while (node) {
        Node* next = reinterpret_cast<Node*>(
            node->load(std::memory_order_relaxed)
        );

        if (node_pool.compare_exchange_weak(node, next,
            std::memory_order_release,
            std::memory_order_relaxed)) {

            new (&node->val) obj(v);
            node->next.store(nullptr, std::memory_order_relaxed);
            return node;
        }
    }

    return new Node(v);
}

template <typename obj>
void lfqueue<obj>::return_to_pool(Node* node) {
    Node* pool_node = node_pool.load(std::memory_order_relaxed);

    do {
        node->next.store(
            reinterpret_cast<Node*>(pool_node), std::memory_order_relaxed
        );
    } while (!node_pool.compare_exchange_weak(pool_node, node,
        std::memory_order_release,
        std::memory_order_relaxed
    ));
}
