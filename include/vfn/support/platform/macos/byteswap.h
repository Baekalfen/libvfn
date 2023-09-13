
// https://gist.github.com/atr000/249599]

#define bswap_16(value) \
((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
<< 32) | \
(uint64_t)bswap_32((uint32_t)((value) >> 32)))



// #include <libkern/OSByteOrder.h>
// #define bswap_16(x) OSSwapInt16(x)
// #define bswap_32(x) OSSwapInt32(x)
// #define bswap_64(x) OSSwapInt64(x)