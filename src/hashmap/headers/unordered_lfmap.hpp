#include <atomic>
#include 

template <typename obj>
namespace hasher {
    __int128 hash (obj input) {
        // encrypt into a int128
    }

    obj unhash () {

        return obj{};
    }
}

class unordered_lfmap {

    struct elem {
        AtomicHashedElem val;
    }

    private:
    public:
}