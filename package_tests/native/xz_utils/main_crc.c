/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xz_utils: crc; calls lzma_crc32(), lzma_crc64().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    static const unsigned char vec[] = "123456789";
    const size_t vec_size = sizeof(vec) - 1;  /* exclude trailing NUL */

    /* ---------- CRC-32 ---------- */
    uint32_t crc32_val = lzma_crc32(vec, vec_size, 0);
    assert(crc32_val == 0xCBF43926u);

    /* Incremental should match one-shot. */
    uint32_t crc32_incr = lzma_crc32(vec, 4, 0);
    crc32_incr = lzma_crc32(vec + 4, vec_size - 4, crc32_incr);
    assert(crc32_incr == crc32_val);

    /* Empty input with seed 0 → still 0. */
    assert(lzma_crc32(vec, 0, 0) == 0u);

    /* ---------- CRC-64 ---------- */
    uint64_t crc64_val = lzma_crc64(vec, vec_size, 0);
    assert(crc64_val == 0x995DC9BBDF1939FAull);

    /* Incremental should match one-shot. */
    uint64_t crc64_incr = lzma_crc64(vec, 5, 0);
    crc64_incr = lzma_crc64(vec + 5, vec_size - 5, crc64_incr);
    assert(crc64_incr == crc64_val);

    static const unsigned char other[] = "123456780";
    uint32_t crc32_other = lzma_crc32(other, sizeof(other) - 1, 0);
    assert(crc32_other != crc32_val);

    printf("xz_utils CRC test OK: crc32=%08x crc64=%016llx\n",
           crc32_val, (unsigned long long)crc64_val);
    return 0;
}
