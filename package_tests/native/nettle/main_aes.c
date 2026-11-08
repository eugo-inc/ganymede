/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nettle: aes; calls aes128_set_encrypt_key(), aes128_encrypt(), aes128_set_decrypt_key(), aes128_decrypt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/aes.h>
#include <nettle/nettle-meta.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t key[AES128_KEY_SIZE] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    };
    static const uint8_t plaintext[AES_BLOCK_SIZE] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    };
    static const uint8_t expected_ct[AES_BLOCK_SIZE] = {
        0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
        0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97,
    };

    struct aes128_ctx enc_ctx;
    uint8_t ciphertext[AES_BLOCK_SIZE];
    aes128_set_encrypt_key(&enc_ctx, key);
    aes128_encrypt(&enc_ctx, AES_BLOCK_SIZE, ciphertext, plaintext);
    assert(memcmp(ciphertext, expected_ct, AES_BLOCK_SIZE) == 0);

    struct aes128_ctx dec_ctx;
    uint8_t decrypted[AES_BLOCK_SIZE];
    aes128_set_decrypt_key(&dec_ctx, key);
    aes128_decrypt(&dec_ctx, AES_BLOCK_SIZE, decrypted, ciphertext);
    assert(memcmp(decrypted, plaintext, AES_BLOCK_SIZE) == 0);

    /* The cipher meta-registry must advertise "aes128". This catches a
     * broken nettle_ciphers table even if the direct API works. */
    const struct nettle_cipher *const *ciphers = nettle_get_ciphers();
    assert(ciphers != NULL);
    int found_aes128 = 0;
    for (const struct nettle_cipher *const *p = ciphers; *p != NULL; ++p) {
        if (strcmp((*p)->name, "aes128") == 0) {
            found_aes128 = 1;
            assert((*p)->block_size == AES_BLOCK_SIZE);
            assert((*p)->key_size == AES128_KEY_SIZE);
            break;
        }
    }
    assert(found_aes128);

    printf("nettle aes test OK\n");
    return 0;
}
