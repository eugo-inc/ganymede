/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zlib_ng: checksum; calls zng_crc32(), zng_adler32().
 */

#include <zlib-ng.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const uint8_t *data = (const uint8_t *)"123456789";
    const uint32_t len = 9;

    const uint32_t expected_crc32   = 0xCBF43926U;
    const uint32_t expected_adler32 = 0x091E01DEU;

    /* One-shot. */
    uint32_t crc = zng_crc32(0, NULL, 0);
    crc = zng_crc32(crc, data, len);
    assert(crc == expected_crc32);

    uint32_t adl = zng_adler32(0, NULL, 0);
    adl = zng_adler32(adl, data, len);
    assert(adl == expected_adler32);

    /* Split into two calls: the running value must match the one-shot result. */
    uint32_t crc_a = zng_crc32(0, NULL, 0);
    crc_a = zng_crc32(crc_a, data, 4);
    crc_a = zng_crc32(crc_a, data + 4, 5);
    assert(crc_a == expected_crc32);

    uint32_t adl_a = zng_adler32(0, NULL, 0);
    adl_a = zng_adler32(adl_a, data, 4);
    adl_a = zng_adler32(adl_a, data + 4, 5);
    assert(adl_a == expected_adler32);

    printf("zlib_ng checksum probe OK: crc32=0x%08lx adler32=0x%08lx\n", (unsigned long)crc, (unsigned long)adl);
    return 0;
}
