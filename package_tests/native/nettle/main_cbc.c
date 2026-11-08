/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nettle: cbc; calls aes128_set_encrypt_key(), aes128_set_decrypt_key(), cbc_aes128_encrypt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/cbc.h>
#include <nettle/aes.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t key[AES128_KEY_SIZE] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    };
    static const uint8_t iv[AES_BLOCK_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };
    static const uint8_t plaintext[2 * AES_BLOCK_SIZE] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    };
    static const uint8_t expected_ct[2 * AES_BLOCK_SIZE] = {
        0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
        0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
        0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
        0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
    };

    /* --- Path A: CBC_CTX + CBC_ENCRYPT/CBC_DECRYPT macro dispatch ---
     * Exercises the function-pointer cbc_encrypt/cbc_decrypt generic core. */
    struct CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE) enc_ctx;
    aes128_set_encrypt_key(&enc_ctx.ctx, key);
    CBC_SET_IV(&enc_ctx, iv);
    uint8_t ciphertext[2 * AES_BLOCK_SIZE];
    CBC_ENCRYPT(&enc_ctx, aes128_encrypt, sizeof(plaintext), ciphertext, plaintext);
    assert(memcmp(ciphertext, expected_ct, sizeof(expected_ct)) == 0);

    struct CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE) dec_ctx;
    aes128_set_decrypt_key(&dec_ctx.ctx, key);
    CBC_SET_IV(&dec_ctx, iv);
    uint8_t decrypted[2 * AES_BLOCK_SIZE];
    CBC_DECRYPT(&dec_ctx, aes128_decrypt, sizeof(ciphertext), decrypted, ciphertext);
    assert(memcmp(decrypted, plaintext, sizeof(plaintext)) == 0);

    /* --- Path B: the dedicated cbc_aes128_encrypt() function must give the same ciphertext --- */
    struct aes128_ctx fast_ctx;
    uint8_t fast_iv[AES_BLOCK_SIZE];
    uint8_t fast_ct[2 * AES_BLOCK_SIZE];
    aes128_set_encrypt_key(&fast_ctx, key);
    memcpy(fast_iv, iv, AES_BLOCK_SIZE);
    cbc_aes128_encrypt(&fast_ctx, fast_iv, sizeof(plaintext), fast_ct, plaintext);
    assert(memcmp(fast_ct, expected_ct, sizeof(expected_ct)) == 0);

    /* A different IV must give a different ciphertext. */
    struct CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE) zero_iv_ctx;
    aes128_set_encrypt_key(&zero_iv_ctx.ctx, key);
    uint8_t zero_iv[AES_BLOCK_SIZE] = {0};
    CBC_SET_IV(&zero_iv_ctx, zero_iv);
    uint8_t alt_ct[2 * AES_BLOCK_SIZE];
    CBC_ENCRYPT(&zero_iv_ctx, aes128_encrypt, sizeof(plaintext), alt_ct, plaintext);
    assert(memcmp(alt_ct, expected_ct, sizeof(expected_ct)) != 0);

    printf("nettle cbc test OK\n");
    return 0;
}
