/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openssl: core API; calls EVP_get_cipherbyname().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char expected[SHA256_DIGEST_LENGTH] = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
    };
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)"hello", 5, digest);
    assert(memcmp(digest, expected, sizeof(digest)) == 0);

    /* AES-256-CBC must be registered with the EVP cipher table: catches a missing cipher registration. */
    const EVP_CIPHER *aes = EVP_get_cipherbyname("AES-256-CBC");
    assert(aes != NULL);

    printf("openssl test OK\n");
    return 0;
}
