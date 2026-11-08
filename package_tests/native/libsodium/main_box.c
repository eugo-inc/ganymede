/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsodium: box; calls sodium_init(), crypto_box_seed_keypair(), crypto_box_easy(), crypto_box_open_easy().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(sodium_init() >= 0);

    unsigned char alice_seed[crypto_box_SEEDBYTES];
    unsigned char bob_seed[crypto_box_SEEDBYTES];
    memset(alice_seed, 0xa1, sizeof(alice_seed));
    memset(bob_seed,   0xb0, sizeof(bob_seed));

    unsigned char alice_pk[crypto_box_PUBLICKEYBYTES], alice_sk[crypto_box_SECRETKEYBYTES];
    unsigned char bob_pk[crypto_box_PUBLICKEYBYTES],   bob_sk[crypto_box_SECRETKEYBYTES];
    assert(crypto_box_seed_keypair(alice_pk, alice_sk, alice_seed) == 0);
    assert(crypto_box_seed_keypair(bob_pk,   bob_sk,   bob_seed)   == 0);

    /* Alice and Bob must end up with distinct public keys (sanity check). */
    assert(memcmp(alice_pk, bob_pk, sizeof(alice_pk)) != 0);

    unsigned char nonce[crypto_box_NONCEBYTES];
    memset(nonce, 0x33, sizeof(nonce));

    static const unsigned char plaintext[] = "demo libsodium crypto_box probe";
    const unsigned long long pt_len = sizeof(plaintext) - 1;

    unsigned char ciphertext[sizeof(plaintext) - 1 + crypto_box_MACBYTES];
    assert(crypto_box_easy(ciphertext, plaintext, pt_len,
                           nonce, bob_pk, alice_sk) == 0);

    /* Cipher must not be a no-op. */
    assert(memcmp(ciphertext, plaintext, pt_len) != 0);

    /* Bob decrypts FROM alice — must succeed. */
    unsigned char recovered[sizeof(plaintext) - 1];
    assert(crypto_box_open_easy(recovered, ciphertext,
                                pt_len + crypto_box_MACBYTES,
                                nonce, alice_pk, bob_sk) == 0);
    assert(memcmp(recovered, plaintext, pt_len) == 0);

    /* Tampered ciphertext must be rejected. */
    unsigned char bad_ct[sizeof(ciphertext)];
    memcpy(bad_ct, ciphertext, sizeof(ciphertext));
    bad_ct[0] ^= 0x01;
    assert(crypto_box_open_easy(recovered, bad_ct,
                                pt_len + crypto_box_MACBYTES,
                                nonce, alice_pk, bob_sk) != 0);

    /* Wrong sender public key must be rejected (the MAC is sender-authenticated). */
    assert(crypto_box_open_easy(recovered, ciphertext,
                                pt_len + crypto_box_MACBYTES,
                                nonce, bob_pk, bob_sk) != 0);

    printf("libsodium crypto_box (X25519) round-trip OK\n");
    return 0;
}
