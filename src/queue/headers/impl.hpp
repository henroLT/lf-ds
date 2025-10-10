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
    Node* newNode = new Node(elem);
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
        std::memory_order_release,
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
            delete firstNode;
            delete first;
            return value;
        }
    }
}

template <typename obj>
lfqueue<obj>::~lfqueue() {
    NodePointer* current = head.ptr.load(std::memory_order_relaxed);

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
}

