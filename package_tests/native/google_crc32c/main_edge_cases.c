/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for google_crc32c: edge cases; calls crc32c_value(), crc32c_extend().
 */

#include <crc32c/crc32c.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* 1. Empty input identity. */
    uint32_t empty = crc32c_value(NULL, 0);
    assert(empty == 0u);

    /* 2. Zero-length extend is a no-op on the running CRC. */
    uint32_t seed = 0xdeadbeefu;
    uint32_t extended_empty = crc32c_extend(seed, NULL, 0);
    assert(extended_empty == seed);

    /* 3. Single byte 'a' (0x61) — published CRC32C reference value. */
    static const uint8_t single = 'a';
    uint32_t one_byte = crc32c_value(&single, 1);
    assert(one_byte == 0xc1d04330u);

    /* 4. Three-way split of the RFC 3720 vector. */
    static const uint8_t input[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    const uint32_t expected = 0xe3069283u;

    uint32_t rolling = crc32c_extend(0, input, 1);
    rolling = crc32c_extend(rolling, input + 1, 4);
    rolling = crc32c_extend(rolling, input + 5, 4);
    assert(rolling == expected);

    printf("google_crc32c edge-cases OK: empty=0x%08x, 'a'=0x%08x, split3=0x%08x\n",
           empty, one_byte, rolling);
    return 0;
}
