/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgcrypt: aes; calls gcry_check_version(), gcry_control(), gcry_cipher_open(), gcry_cipher_setkey().
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

    /* AES-128-CBC: 16-byte key, 16-byte IV, plaintext must be block-aligned. */
    static const unsigned char key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };
    static const unsigned char iv[16] = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    static const unsigned char plaintext[16] = "Demo-libgcrypt!!";
    unsigned char ciphertext[16];
    unsigned char roundtrip[16];

    gcry_cipher_hd_t hd;
    gcry_error_t err;

    err = gcry_cipher_open(&hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, 0);
    assert(err == 0);
    err = gcry_cipher_setkey(hd, key, sizeof(key));
    assert(err == 0);
    err = gcry_cipher_setiv(hd, iv, sizeof(iv));
    assert(err == 0);
    err = gcry_cipher_encrypt(hd, ciphertext, sizeof(ciphertext), plaintext, sizeof(plaintext));
    assert(err == 0);
    /* Ciphertext MUST differ from plaintext — otherwise the cipher is a no-op. */
    assert(memcmp(ciphertext, plaintext, sizeof(plaintext)) != 0);

    /* Re-key + re-IV the same handle to decrypt. */
    err = gcry_cipher_reset(hd);
    assert(err == 0);
    err = gcry_cipher_setkey(hd, key, sizeof(key));
    assert(err == 0);
    err = gcry_cipher_setiv(hd, iv, sizeof(iv));
    assert(err == 0);
    err = gcry_cipher_decrypt(hd, roundtrip, sizeof(roundtrip), ciphertext, sizeof(ciphertext));
    assert(err == 0);
    assert(memcmp(roundtrip, plaintext, sizeof(plaintext)) == 0);

    gcry_cipher_close(hd);
    printf("libgcrypt AES test OK\n");
    return 0;
}
