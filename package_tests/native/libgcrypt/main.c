/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgcrypt: core API; calls gcry_check_version(), gcry_control(), gcry_md_get_algo_dlen(), gcry_md_hash_buffer().
 */

#include <gcrypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* gcry_check_version MUST be called before any other libgcrypt function — passing NULL
     * skips the minimum-version check but still performs required initialization. */
    const char *v = gcry_check_version(NULL);
    assert(v != NULL);

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    static const unsigned char expected[32] = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
    };
    unsigned int dlen = gcry_md_get_algo_dlen(GCRY_MD_SHA256);
    assert(dlen == 32);

    unsigned char digest[32];
    gcry_md_hash_buffer(GCRY_MD_SHA256, digest, "hello", 5);
    assert(memcmp(digest, expected, sizeof(digest)) == 0);

    printf("libgcrypt test OK: %s\n", v);
    return 0;
}
