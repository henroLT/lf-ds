#pragma once

#include "lfqueue.hpp"
#include <atomic>
#include <cstdint>
#include <cstring>

template <typename T>
struct PointerWrapper {
    T* node_ptr;
    uint64_t cnt;

    PointerWrapper() : node_ptr(nullptr), cnt(0) {}
    PointerWrapper(T* node) : node_ptr(node), cnt(0) {}
    PointerWrapper(T* node, uint64_t c) : node_ptr(node), cnt(c) {}

    ~PointerWrapper() = default;

} __attribute__ ((packed, aligned(16)));

template <typename T>
inline __int128 pack (const PointerWrapper<T>& p) {
    static_assert(sizeof(PointerWrapper<T>) <= sizeof(__int128));
    __int128 v = 0;
    std::memcpy(&v, &p, sizeof(p));

    return v;
}

template <typename T>
inline PointerWrapper<T> unpack (__int128 v) {
    PointerWrapper<T> p;
    std::memcpy(&p, &v, sizeof(p));

    return p;
}


template <typename T>
class AtomicPointerWrapper {

    private:
        alignas(16) std::atomic<__int128> raw;

    public:
        AtomicPointerWrapper() { raw.store (0, std::memory_order_relaxed); }

        void store (const PointerWrapper<T>& p, std::memory_order order = std::memory_order_seq_cst) {
            raw.store(pack(p), order);
        }
        
        PointerWrapper<T> load (std::memory_order order = std::memory_order_seq_cst) const {
            return unpack<T>(raw.load(order));
        }

        bool compare_exchange_weak (PointerWrapper<T>& expected, const PointerWrapper<T>& desired,
            std::memory_order succ, std::memory_order fail) {
            
            __int128 val = pack(expected);
            bool completed = raw.compare_exchange_weak (val, pack(desired), succ, fail);
            if (!completed) expected = unpack<T>(val);

            return completed;
        }

        bool compare_exchange_strong (PointerWrapper<T>& expected, const PointerWrapper<T>& desired,
            std::memory_order succ, std::memory_order fail) {
            
            __int128 val = pack (expected);
            bool completed = raw.compare_exchange_strong (val, pack(desired), succ, fail);
            if (!completed) expected = unpack<T>(val);

            return completed;
        }

        AtomicPointerWrapper (const AtomicPointerWrapper&) = delete;
        AtomicPointerWrapper& operator=(const AtomicPointerWrapper&) = delete;

    
};