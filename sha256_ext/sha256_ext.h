#ifndef SHA256_EXT_H
#define SHA256_EXT_H

#include <cstdint>

// SHA256 with SHA Extensions for 8 blocks in parallel
void sha256sha_8B(
    const uint8_t* data0, const uint8_t* data1, const uint8_t* data2, const uint8_t* data3,
    const uint8_t* data4, const uint8_t* data5, const uint8_t* data6, const uint8_t* data7,
    unsigned char* hash0, unsigned char* hash1, unsigned char* hash2, unsigned char* hash3,
    unsigned char* hash4, unsigned char* hash5, unsigned char* hash6, unsigned char* hash7
);

#endif // SHA256_SHA_H

