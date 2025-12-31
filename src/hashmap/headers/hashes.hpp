#pragma once

#include "consts.hpp"
#include <cstdint>
#include <cstddef>
#include <functional>
#include <type_traits>

namespace hashes {

    /* SplitMix64 hash */
    namespace detail {
        static inline constexpr uint64_t splitmix (uint64_t x) noexcept {
            x += 0x9e3779b97f4a7c15ULL;
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
            x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
            return x ^ (x >> 31);
        }
    }

    


};
