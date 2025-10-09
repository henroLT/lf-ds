#include <atmoic>

template <typename T>
class lfqueue {

    /* ========================== Nested Structs ============================ */
    struct nodePointer;

    /* Immutable payload */
    struct Node {
        const T                     val;
        std::atomic<nodePointer*>   next;

        Node()                      : val(T{}), next(nullptr) {}
        Node(T v)                   : val(v), next(nullptr) {}
        Node(T v, nodePointer* n)   : val(v), next(n) {}
        ~Node()                     = default;
    };

    struct nodePointer {
        std::atomic<Node*>      node;
        std::atomic<unsigned>   count;

        nodePointer()                       : node(nullptr), count(0) {}
        nodePointer(Node* n)                : node(n), count(0) {}
        nodePointer(Node* n, unsigned cnt)  : node(n), count(cnt) {}
        ~nodePointer()                      = default;
    };

    /* =========================== Private Members ===========================*/
    private:
        std::atomic<nodePointer> head;
        std::atomic<nodePointer> tail;

        Node* node_init(const &T val) {
            Node* temp = new Node;
            temp -> val = val;
            temp -> next.store(NULL);

            return temp;
        }

    /* ========================== Public Interface ===========================*/
    public:
        lfqueue() {
            Node* dummy = node_init(T{});
            nodePointer* dummyPtr = new nodePointer(dummy, 0);
            head.store(dummyPtr);
            tail.store(dummyPtr);
        }

        void push(const T &elem) {

        }

        T pop() {

        }

        ~lfqueue() {

        }
        
}