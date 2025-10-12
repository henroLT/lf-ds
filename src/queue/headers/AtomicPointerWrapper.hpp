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

    public:
        void store (const PointerWrapper& p) {
            raw.store(pack(p), std::memory_order_seq_cst);
        }
        
        PointerWrapper load () const {
            return unpack(raw.load(std::memory_order_seq_cst));
        }

        bool compare_exchange_weak (PointerWrapper& expected, const PointerWrapper& desired,
            std::memory_order succ, std::memory_order fail) {
            
            __int128 val = pack (expected);
            bool completed = raw.compare_exchange_weak (val, pack(desired), succ, fail);
            if (!completed) expected = unpack (val);

            return completed;
        }

        bool compare_exchange_strong (PointerWrapper& expected, const PointerWrapper& desired,
            std::memory_order succ, std::memory_order fail) {
            
            __int128 val = pack (expected);
            bool completed = raw.compare_exchange_strong (val, pack(desired), succ, fail);
            if (!completed) expected = unpack (val);

            return completed;
        }
    
}