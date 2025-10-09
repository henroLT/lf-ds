#include <atmoic>

template <typename T>
class lfqueue {

    /* ========================== Nested Structs ============================ */
    struct nodePointer;

    struct node {
        const T val;
        std::atomic<nodePointer*> next;
    };

    struct nodePointer {
        std::atomic<node<T>*> nodeP;
        std::atomic<unsigned> count;

        ~nodePointer() = default;
    };

    private:
    std::atomic<nodePointer> head;
    std::atomic<nodePointer> tail;

    node<T>* node_init(const &T val) {
        node<T>* temp = new node<T>;
        temp -> val.store(val);
        temp -> next.store(NULL);

        return temp;
    }

    public:
        lfqueue();

        void push(const T &elem) {

        }

        T pop() {

        }

        ~lfqueue() {

        }
        
}