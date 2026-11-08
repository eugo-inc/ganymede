/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for minizip_ng: crypt; calls mz_crypt_crc32_update().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_crypt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    const char *msg = "123456789";
    uint32_t crc = mz_crypt_crc32_update(0, (const uint8_t *)msg, (int32_t)strlen(msg));
    if (crc != 0xCBF43926u) {
        fprintf(stderr, "minizip_ng: CRC32 mismatch — got 0x%08x, want 0xcbf43926\n", crc);
        return 1;
    }

    /* Multi-chunk update must match single-shot. */
    uint32_t crc_a = mz_crypt_crc32_update(0, (const uint8_t *)msg, 4);
    uint32_t crc_b = mz_crypt_crc32_update(crc_a, (const uint8_t *)msg + 4, (int32_t)(strlen(msg) - 4));
    assert(crc_b == 0xCBF43926u);

    printf("minizip_ng crypt CRC32 OK\n");
    return 0;
}
