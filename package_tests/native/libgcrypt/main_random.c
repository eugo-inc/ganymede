/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgcrypt: random; calls gcry_check_version(), gcry_control(), gcry_randomize(), gcry_create_nonce().
 */

#include <gcrypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *v = gcry_check_version(NULL);
    assert(v != NULL);
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    /* gcry_randomize: GCRY_STRONG_RANDOM path. */
    unsigned char buf1[64];
    unsigned char buf2[64];
    unsigned char zero[64];
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    memset(zero, 0, sizeof(zero));

    gcry_randomize(buf1, sizeof(buf1), GCRY_STRONG_RANDOM);
    gcry_randomize(buf2, sizeof(buf2), GCRY_STRONG_RANDOM);

    assert(memcmp(buf1, zero, sizeof(buf1)) != 0);
    assert(memcmp(buf2, zero, sizeof(buf2)) != 0);
    assert(memcmp(buf1, buf2, sizeof(buf1)) != 0);

    /* gcry_create_nonce: a separate, faster nonce path. */
    unsigned char nonce[32];
    memset(nonce, 0, sizeof(nonce));
    gcry_create_nonce(nonce, sizeof(nonce));
    assert(memcmp(nonce, zero, sizeof(nonce)) != 0);

    printf("libgcrypt RNG test OK\n");
    return 0;
}
