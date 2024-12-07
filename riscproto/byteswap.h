#ifndef _BYTESWAP_
#define _BYTESWAP_

#include <stdint.h>

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define __CSX_LITTLE_ENDIAN__
#endif

static inline uint16_t bswap_16(uint16_t x) {
    return (x >> 8) | (x << 8);
}

static inline uint32_t bswap_32(uint32_t x) {
    return ((x & 0xFF000000) >> 24)
        | ((x & 0xFF0000) >> 8)
        | ((x & 0xFF00) << 8)
        | ((x & 0xFF) << 24);
}

static inline uint64_t bswap_64(uint64_t x) {
    return ((x & 0xFF00000000000000) >> 56)
        | ((x & 0xFF000000000000) >> 40)
        | ((x & 0xFF0000000000) >> 24)
        | ((x & 0xFF00000000) >> 8)
        | ((x & 0xFF000000) << 8)
        | ((x & 0xFF0000) << 24)
        | ((x & 0xFF00) << 40)
        | ((x & 0xFF) << 56);
}

#endif

