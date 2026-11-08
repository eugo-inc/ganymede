/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsodium: sign; calls sodium_init(), crypto_sign_seed_keypair(), crypto_sign_detached(), crypto_sign_verify_detached().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(sodium_init() >= 0);

    unsigned char seed[crypto_sign_SEEDBYTES];
    memset(seed, 0x42, sizeof(seed));

    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    assert(crypto_sign_seed_keypair(pk, sk, seed) == 0);

    static const unsigned char msg[] = "demo libsodium ed25519 probe";
    const size_t msg_len = sizeof(msg) - 1;

    unsigned char sig[crypto_sign_BYTES];
    unsigned long long sig_len = 0;
    assert(crypto_sign_detached(sig, &sig_len, msg, msg_len, sk) == 0);
    assert(sig_len == crypto_sign_BYTES);

    /* Honest signature must verify. */
    assert(crypto_sign_verify_detached(sig, msg, msg_len, pk) == 0);

    /* Tampered signature must NOT verify (proves verify is not a no-op). */
    unsigned char bad_sig[crypto_sign_BYTES];
    memcpy(bad_sig, sig, sizeof(bad_sig));
    bad_sig[0] ^= 0x01;
    assert(crypto_sign_verify_detached(bad_sig, msg, msg_len, pk) != 0);

    /* Tampered message must NOT verify under the honest signature either. */
    unsigned char bad_msg[sizeof(msg) - 1];
    memcpy(bad_msg, msg, msg_len);
    bad_msg[0] ^= 0x01;
    assert(crypto_sign_verify_detached(sig, bad_msg, msg_len, pk) != 0);

    printf("libsodium Ed25519 sign+verify round-trip OK\n");
    return 0;
}
