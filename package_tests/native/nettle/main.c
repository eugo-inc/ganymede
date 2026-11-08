/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nettle: core API; calls sha256_init(), sha256_update(), sha256_digest(), nettle_get_hashes().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/sha2.h>
#include <nettle/nettle-meta.h>
#include <nettle/version.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t expected[SHA256_DIGEST_SIZE] = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
    };
    struct sha256_ctx ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    sha256_init(&ctx);
    sha256_update(&ctx, 5, (const uint8_t *)"hello");
    /* The digest signature differs between nettle 3 and 4. */
#if NETTLE_VERSION_MAJOR >= 4
    sha256_digest(&ctx, digest);
#else
    sha256_digest(&ctx, SHA256_DIGEST_SIZE, digest);
#endif
    assert(memcmp(digest, expected, sizeof(digest)) == 0);

    /* Walk the hash meta-algorithm registry; "sha256" must be registered. */
    const struct nettle_hash *const *hashes = nettle_get_hashes();
    assert(hashes != NULL);
    int found_sha256 = 0;
    for (const struct nettle_hash *const *p = hashes; *p != NULL; ++p) {
        if (strcmp((*p)->name, "sha256") == 0) {
            found_sha256 = 1;
            break;
        }
    }
    assert(found_sha256);

    printf("nettle test OK\n");
    return 0;
}
