#pragma once

#include "lfqueue.hpp"
#include <atomic>
#include <cstdint>
#include <cstring>

class lfqueue();

struct PointerWrapper {
    Node* node_ptr;
    uint64_t cnt;
} __attribute__ ((packed, aligned(16)));

inline __int128 pack (const PointerWrapper& p) {
    __int128 v = 0;
    std::memcpy(&v, &p, sizeof(p));

    return v;
}

inline PointerWrapper unpack (__int128 v) {
    PointerWrapper p;
    std::memcpy(&p, &v, sizeof(p));

    return p;
}


class AtomicPointerWrapper {
    private:
        alignas(16) std::atomic<__int128> raw;
    
}