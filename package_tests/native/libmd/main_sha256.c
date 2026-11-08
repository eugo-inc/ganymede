/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmd: sha256; calls SHA256Init(), SHA256Update(), SHA256Final().
 */

#include <sha2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char expected_sha256[SHA256_DIGEST_LENGTH] = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
    };
    unsigned char sha256_digest[SHA256_DIGEST_LENGTH];
    SHA2_CTX sha256_ctx;
    SHA256Init(&sha256_ctx);
    SHA256Update(&sha256_ctx, (const unsigned char *)"hello", 5);
    SHA256Final(sha256_digest, &sha256_ctx);
    assert(memcmp(sha256_digest, expected_sha256, sizeof(sha256_digest)) == 0);

    /* SHA-256 of the empty input is a canonical known constant
     * (e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855).
     * Verifying an empty-input flow catches Init-state initialization bugs that
     * a populated buffer would mask. */
    static const unsigned char expected_sha256_empty[SHA256_DIGEST_LENGTH] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
    };
    unsigned char sha256_empty[SHA256_DIGEST_LENGTH];
    SHA2_CTX sha256_ctx_empty;
    SHA256Init(&sha256_ctx_empty);
    SHA256Final(sha256_empty, &sha256_ctx_empty);
    assert(memcmp(sha256_empty, expected_sha256_empty, sizeof(sha256_empty)) == 0);

    printf("libmd sha256 test OK\n");
    return 0;
}
