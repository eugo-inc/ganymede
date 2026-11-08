/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nettle: hmac; calls hmac_sha256_set_key(), hmac_sha256_update(), hmac_sha256_digest().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/hmac.h>
#include <nettle/sha2.h>
#include <nettle/version.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* The digest signature differs between nettle 3 and 4. */
#if NETTLE_VERSION_MAJOR >= 4
#define HMAC_DIGEST(c, d) hmac_sha256_digest((c), (d))
#else
#define HMAC_DIGEST(c, d) hmac_sha256_digest((c), SHA256_DIGEST_SIZE, (d))
#endif

int main(void) {
    static const uint8_t key[20] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b,
    };
    static const char data[] = "Hi There";
    static const uint8_t expected[SHA256_DIGEST_SIZE] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
    };

    struct hmac_sha256_ctx ctx;
    uint8_t mac[SHA256_DIGEST_SIZE];

    hmac_sha256_set_key(&ctx, sizeof(key), key);
    hmac_sha256_update(&ctx, sizeof(data) - 1, (const uint8_t *)data);
    HMAC_DIGEST(&ctx, mac);
    assert(memcmp(mac, expected, SHA256_DIGEST_SIZE) == 0);

    /* Re-running on the same context (after digest) must reset internal
     * state and produce the same MAC for the same data — catches a
     * broken hmac_sha256 state-reset path. */
    uint8_t mac2[SHA256_DIGEST_SIZE];
    hmac_sha256_update(&ctx, sizeof(data) - 1, (const uint8_t *)data);
    HMAC_DIGEST(&ctx, mac2);
    assert(memcmp(mac2, expected, SHA256_DIGEST_SIZE) == 0);

    uint8_t mac_other[SHA256_DIGEST_SIZE];
    hmac_sha256_update(&ctx, 7, (const uint8_t *)"goodbye");
    HMAC_DIGEST(&ctx, mac_other);
    assert(memcmp(mac_other, expected, SHA256_DIGEST_SIZE) != 0);

    printf("nettle hmac test OK\n");
    return 0;
}
