/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsodium: aead; calls sodium_init(), crypto_aead_xchacha20poly1305_ietf_encrypt(), crypto_aead_xchacha20poly1305_ietf_decrypt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(sodium_init() >= 0);

    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    memset(key, 0x11, sizeof(key));
    memset(nonce, 0x22, sizeof(nonce));

    static const unsigned char plaintext[] = "demo libsodium aead probe";
    const unsigned long long pt_len = sizeof(plaintext) - 1;
    static const unsigned char ad[] = "metadata";
    const unsigned long long ad_len = sizeof(ad) - 1;

    unsigned char ciphertext[sizeof(plaintext) - 1 + crypto_aead_xchacha20poly1305_ietf_ABYTES];
    unsigned long long ct_len = 0;

    assert(crypto_aead_xchacha20poly1305_ietf_encrypt(
        ciphertext, &ct_len,
        plaintext, pt_len,
        ad, ad_len,
        NULL, nonce, key) == 0);
    assert(ct_len == pt_len + crypto_aead_xchacha20poly1305_ietf_ABYTES);

    assert(memcmp(ciphertext, plaintext, pt_len) != 0);

    /* Honest decrypt must succeed and recover the plaintext. */
    unsigned char recovered[sizeof(plaintext) - 1];
    unsigned long long rec_len = 0;
    assert(crypto_aead_xchacha20poly1305_ietf_decrypt(
        recovered, &rec_len, NULL,
        ciphertext, ct_len,
        ad, ad_len,
        nonce, key) == 0);
    assert(rec_len == pt_len);
    assert(memcmp(recovered, plaintext, pt_len) == 0);

    /* Tampered ciphertext must be rejected by the Poly1305 tag check. */
    unsigned char bad_ct[sizeof(ciphertext)];
    memcpy(bad_ct, ciphertext, ct_len);
    bad_ct[0] ^= 0x01;
    assert(crypto_aead_xchacha20poly1305_ietf_decrypt(
        recovered, &rec_len, NULL,
        bad_ct, ct_len,
        ad, ad_len,
        nonce, key) != 0);

    /* Wrong associated data must also be rejected (AD is bound to the tag). */
    static const unsigned char wrong_ad[] = "tampered";
    assert(crypto_aead_xchacha20poly1305_ietf_decrypt(
        recovered, &rec_len, NULL,
        ciphertext, ct_len,
        wrong_ad, sizeof(wrong_ad) - 1,
        nonce, key) != 0);

    printf("libsodium XChaCha20-Poly1305-IETF AEAD round-trip OK\n");
    return 0;
}
