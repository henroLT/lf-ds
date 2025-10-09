#include <atomic>

template <typename obj>
class lfqueue {

    /* ========================== Nested Structs ============================ */
    struct NodePointer;
    struct Node;
    struct PointerWrapper;

    /* Immutable payload */
    struct Node {
        const obj val;
        std::atomic<NodePointer*> next;

        Node()                      : val(obj{}), next(nullptr) {}
        Node(obj v)                   : val(v), next(nullptr) {}
        Node(obj v, NodePointer* n)   : val(v), next(n) {}
        ~Node()                     = default;
    };

    struct NodePointer {
        std::atomic<Node*> node;
        std::atomic<unsigned> count;

        NodePointer()                       : node(nullptr), count(0) {}
        NodePointer(Node* n)                : node(n), count(0) {}
        NodePointer(Node* n, unsigned cnt)  : node(n), count(cnt) {}
        ~NodePointer()                      = default;
    };

    /* Prevent false sharing */
    struct alignas(64) PointerWrapper {
        std::atomic<NodePointer*> ptr;
        char padding[64 - sizeof(std::atomic<NodePointer*>)];
    };



    /* =========================== Private Members ===========================*/
    private:
        PointerWrapper head;
        PointerWrapper tail;

    /* ========================== Public Interface ===========================*/
    public:
        lfqueue() {
            Node* dummy = node_init(obj{});
            NodePointer* dummyPtr = new NodePointer(dummy, 0);
            head.ptr.store(dummyPtr);
            tail.ptr.store(dummyPtr);
        }

        void push(const obj &elem) {
            Node* newNode = new Node(elem);
            NodePointer* newPtr = new NodePointer(newNode);

            while (true) {
                NodePointer* last = tail.ptr.load(std::memory_order_acquire);
                Node* lastNode = last->node.load(std::memory_order_acquire);
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

            NodePointer* last = tail.ptr.load(std::memory_order_acquire);
            while (!tail.ptr.compare_exchange_weak(
                last, newPtr,
                std::memory_order_release,
                std::memory_order_relaxed
            )){}
        }

        obj pop() {

            return obj{};
        }


        ~lfqueue() {
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

};