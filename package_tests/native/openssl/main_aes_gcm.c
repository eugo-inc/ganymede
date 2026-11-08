/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openssl: aes gcm; calls EVP_CIPHER_CTX_new(), EVP_EncryptInit_ex(), EVP_aes_256_gcm(), EVP_CIPHER_CTX_ctrl().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openssl/evp.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    static const unsigned char iv[12] = {
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
        0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    };
    static const unsigned char aad[6] = { 'h', 'e', 'a', 'd', 'e', 'r' };
    static const unsigned char plaintext[13] = "hello, world!";

    unsigned char ciphertext[sizeof(plaintext)];
    unsigned char tag[16];
    int outlen = 0;
    int total = 0;
    int rc = 0;

    /* --- Encrypt --- */
    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    assert(ectx != NULL);
    rc = EVP_EncryptInit_ex(ectx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    assert(rc == 1);
    rc = EVP_CIPHER_CTX_ctrl(ectx, EVP_CTRL_GCM_SET_IVLEN, (int)sizeof(iv), NULL);
    assert(rc == 1);
    rc = EVP_EncryptInit_ex(ectx, NULL, NULL, key, iv);
    assert(rc == 1);
    rc = EVP_EncryptUpdate(ectx, NULL, &outlen, aad, (int)sizeof(aad));
    assert(rc == 1);
    rc = EVP_EncryptUpdate(ectx, ciphertext, &outlen, plaintext, (int)sizeof(plaintext));
    assert(rc == 1);
    total = outlen;
    rc = EVP_EncryptFinal_ex(ectx, ciphertext + outlen, &outlen);
    assert(rc == 1);
    total += outlen;
    assert(total == (int)sizeof(plaintext));
    rc = EVP_CIPHER_CTX_ctrl(ectx, EVP_CTRL_GCM_GET_TAG, (int)sizeof(tag), tag);
    assert(rc == 1);
    EVP_CIPHER_CTX_free(ectx);

    /* Ciphertext must NOT equal plaintext (catches a no-op cipher). */
    assert(memcmp(ciphertext, plaintext, sizeof(plaintext)) != 0);

    /* --- Decrypt --- */
    unsigned char recovered[sizeof(plaintext)];
    EVP_CIPHER_CTX *dctx = EVP_CIPHER_CTX_new();
    assert(dctx != NULL);
    rc = EVP_DecryptInit_ex(dctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    assert(rc == 1);
    rc = EVP_CIPHER_CTX_ctrl(dctx, EVP_CTRL_GCM_SET_IVLEN, (int)sizeof(iv), NULL);
    assert(rc == 1);
    rc = EVP_DecryptInit_ex(dctx, NULL, NULL, key, iv);
    assert(rc == 1);
    rc = EVP_DecryptUpdate(dctx, NULL, &outlen, aad, (int)sizeof(aad));
    assert(rc == 1);
    rc = EVP_DecryptUpdate(dctx, recovered, &outlen, ciphertext, (int)sizeof(ciphertext));
    assert(rc == 1);
    total = outlen;
    rc = EVP_CIPHER_CTX_ctrl(dctx, EVP_CTRL_GCM_SET_TAG, (int)sizeof(tag), tag);
    assert(rc == 1);
    rc = EVP_DecryptFinal_ex(dctx, recovered + outlen, &outlen);
    assert(rc == 1);
    total += outlen;
    EVP_CIPHER_CTX_free(dctx);

    assert(total == (int)sizeof(plaintext));
    assert(memcmp(recovered, plaintext, sizeof(plaintext)) == 0);

    printf("openssl AES-256-GCM round-trip OK\n");
    return 0;
}
