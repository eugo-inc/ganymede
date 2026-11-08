/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liblzf: incompressible; calls lzf_compress(), lzf_decompress().
 */

#include <lzf.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* ---- Aspect 1: incompressible input + tight output buffer → returns 0. ---- */
    /* xorshift32 PRNG — high entropy, no compressible structure. */
    unsigned char random_input[1024];
    uint32_t state = 0xDEADBEEFu;
    for (size_t i = 0; i < sizeof(random_input); ++i) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        random_input[i] = (unsigned char)(state & 0xFF);
    }

    /* LZF documents worst-case output of ~104% of input; a buffer equal to
     * input_len - 1 forces the "doesn't fit" branch for incompressible data. */
    unsigned char tight_out[sizeof(random_input) - 1];
    unsigned int compress_rc = lzf_compress(
        random_input, (unsigned int)sizeof(random_input),
        tight_out, (unsigned int)sizeof(tight_out));
    assert(compress_rc == 0);

    unsigned char binary_input[1023];  /* Deliberately odd, not 8-aligned. */
    for (size_t i = 0; i < sizeof(binary_input); ++i) {
        binary_input[i] = (unsigned char)(i % 17);  /* Includes 0x00 every 17 bytes. */
    }

    unsigned char compressed[2048];
    unsigned int compressed_len = lzf_compress(
        binary_input, (unsigned int)sizeof(binary_input),
        compressed, (unsigned int)sizeof(compressed));
    assert(compressed_len != 0);
    assert(compressed_len < sizeof(binary_input));  /* The mod-17 pattern is compressible. */

    unsigned char decompressed[2048] = {0};
    unsigned int decompressed_len = lzf_decompress(
        compressed, compressed_len, decompressed, (unsigned int)sizeof(decompressed));
    assert(decompressed_len == (unsigned int)sizeof(binary_input));
    assert(memcmp(decompressed, binary_input, sizeof(binary_input)) == 0);

    printf("liblzf incompressible/binary test OK\n");
    return 0;
}
