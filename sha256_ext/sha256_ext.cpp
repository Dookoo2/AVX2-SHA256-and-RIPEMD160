#include "sha256_ext.h"
#include <immintrin.h>
#include <shaintrin.h>
#include <string.h>
#include <stdint.h>

// Initial SHA-256 constants
const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256sha_8B(
    const uint8_t* data0, const uint8_t* data1, const uint8_t* data2, const uint8_t* data3,
    const uint8_t* data4, const uint8_t* data5, const uint8_t* data6, const uint8_t* data7,
    unsigned char* hash0, unsigned char* hash1, unsigned char* hash2, unsigned char* hash3,
    unsigned char* hash4, unsigned char* hash5, unsigned char* hash6, unsigned char* hash7) {

    // Initialize state
    __m128i state[8] = {
        _mm_set1_epi32(0x6a09e667), _mm_set1_epi32(0xbb67ae85),
        _mm_set1_epi32(0x3c6ef372), _mm_set1_epi32(0xa54ff53a),
        _mm_set1_epi32(0x510e527f), _mm_set1_epi32(0x9b05688c),
        _mm_set1_epi32(0x1f83d9ab), _mm_set1_epi32(0x5be0cd19)
    };

    const uint8_t* data[8] = { data0, data1, data2, data3, data4, data5, data6, data7 };
    __m128i W[16];

    // Prepare message schedule
    for (int t = 0; t < 16; ++t) {
        W[t] = _mm_set_epi32(
            ((uint32_t*)data[7])[t], ((uint32_t*)data[6])[t],
            ((uint32_t*)data[5])[t], ((uint32_t*)data[4])[t]);
    }

    for (int t = 16; t < 64; ++t) {
        W[t % 16] = _mm_sha256msg1_epu32(W[(t - 14) % 16], W[(t - 16) % 16]);
        W[t % 16] = _mm_add_epi32(W[t % 16], _mm_sha256msg2_epu32(W[(t - 9) % 16], W[(t - 8) % 16]));
    }
    // Add rounds using _mm_sha256rnds2_epu32...
}

