#pragma once

#define SYS_BITS sizeof(void*)

#if defined(__SIZEOF_INT128__)
    using uint128_t = unsigned __int128;
#else
    struct uint128_t {
        uint64_t hi;
        uint64_t lo;
    };
#endif