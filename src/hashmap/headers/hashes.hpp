#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <type_traits>

namespace hashes {

    /* uint 128 */
    #if defined(__SIZEOF_INT128__)
        using uint128_t = unsigned __int128;

        static inline uint128_t make_u128 (uint64_t hi, uint64_t lo) noexcept {
            return (static_cast<uint128_t> (hi) << 64) | (static_cast<uint128_t> (lo));
        }

        static inline constexpr uint64_t u128_lo (uint128_t v) noexcept { return static_cast<uint64_t> (v); }
        static inline constexpr uint64_t u128_hi (uint128_t v) noexcept { return static_cast<uint64_t> (v >> 64); }
    #else
        struct uint128_t {
            uint64_t hi;
            uint64_t lo;
        };

        static inline uint128_t make_u128 (uint64_t hi, uint64_t lo) noexcept {
            return uint128_t{hi, lo};
        }

        static inline bool operator==(const uint128_t& a, const uint128_t& b) noexcept {
            return a.hi == b.hi && a.lo == b.lo;
        }

        static inline bool operator!=(const uint128_t& a, const uint128_t& b) noexcept {
            return !(a == b);
        }

        static inline constexpr uint64_t u128_hi (uint128_t v) noexcept { return v.hi; }
        static inline constexpr uint64_t u128_lo (uint128_t v) noexcept { return v.lo; }
    #endif


    /* SplitMix64 hash */
    namespace detail {
        
        static inline constexpr uint64_t fold64 (uint64_t x) noexcept { return x ^ (x >> 32); }
        static inline constexpr uint64_t splitmix64 (uint64_t x) noexcept {
            x += 0x9e3779b97f4a7c15ULL;
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
            x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
            return x ^ (x >> 31);
        }
    }


    /* Hashes */
    template <class T>
    static inline uint128_t h128 (const T& input)
        noexcept (noexcept (std::hash<std::decay_t<T>>{} (input)))
    {
        using U = std::decay_t<T>;
        const size_t s0 = std::hash<U>{} (input);

        uint64_t x = static_cast<uint64_t> (s0);
        if constexpr (sizeof (size_t) < sizeof (uint64_t))
            x |= (x << 32);
        
        const uint64_t lo = detail::splitmix64 (x ^ 0x243F6A8885A308D3ULL);
        const uint64_t hi = detail::splitmix64 (x ^ 0x13198A2E03707344ULL);

        return make_u128 (hi, lo);
    }

    template <class T>
    static inline uint64_t h64 (const T& input) noexcept (noexcept (h128 (input))){
        const uint128_t h = h128 (input);
        return detail::splitmix64 (u128_lo(h) ^ detail::splitmix64 (u128_hi(h)));
    }

    template <class T>
    static inline uint32_t h32 (const T& input) noexcept (noexcept (h64 (input))) {
        return  static_cast<uint32_t> (detail::fold64 (h64 (input)));
    }

    template <class T>
    static inline uint16_t h16 (const T& input) noexcept (noexcept (h32 (input))) {
        uint32_t x = h32 (input);
        return static_cast<uint16_t> ((x ^ (x >> 16)));
    }

    template <class T>
    static inline uint8_t h8 (const T& input) noexcept (noexcept (h16 (input))) {
        uint16_t x = h16 (input);
        return static_cast<uint8_t> ((x ^ (x >> 8)));
    }
}
