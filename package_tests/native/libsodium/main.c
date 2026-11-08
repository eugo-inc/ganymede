/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsodium: core API; calls sodium_init(), crypto_generichash(), sodium_version_string().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* sodium_init: 0 = freshly initialized, 1 = already initialized (both OK); -1 = failure. */
    assert(sodium_init() >= 0);

    static const unsigned char expected[32] = {
        0x32, 0x4d, 0xcf, 0x02, 0x7d, 0xd4, 0xa3, 0x0a,
        0x93, 0x2c, 0x44, 0x1f, 0x36, 0x5a, 0x25, 0xe8,
        0x6b, 0x17, 0x3d, 0xef, 0xa4, 0xb8, 0xe5, 0x89,
        0x48, 0x25, 0x34, 0x71, 0xb8, 0x1b, 0x72, 0xcf,
    };
    unsigned char digest[32];
    assert(crypto_generichash(digest, sizeof(digest),
                              (const unsigned char *)"hello", 5,
                              NULL, 0) == 0);
    assert(memcmp(digest, expected, sizeof(digest)) == 0);

    const char *v = sodium_version_string();
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("libsodium test OK: %s\n", v);
    return 0;
}
