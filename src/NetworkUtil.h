#ifndef __NETWORK_UTIL_H__
#define __NETWORK_UTIL_H__

#include <stdint.h>

#ifdef ARDUINO
#define __LITTLE_ENDIAN 1234
#define BYTE_ORDER LITTLE_ENDIAN
#endif // ARDUINO

#if BYTE_ORDER != LITTLE_ENDIAN

int32_t swap_int32(int32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

int64_t swap_int64(int64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

#ifndef htole32
#define htole32(val) swap_int32(val)
#define letoh32(val) swap_int32(val)
#endif // htole32

#ifndef htole64
#define htole64(val) swap_int64(val)
#define letoh64(val) swap_int64(val)
#endif // htole64

#else // BYTE_ORDER != LITTLE_ENDIAN

#ifndef htole32
#define htole32(val) (val)
#define letoh32(val) (val)
#endif // htole32

#ifndef htole64
#define htole64(val) (val)
#define letoh64(val) (val)
#endif // htole64

#endif // BYTE_ORDER != LITTLE_ENDIAN

#endif // __NETWORK_UTIL_H__
