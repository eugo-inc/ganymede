/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nettle: chacha; calls chacha_set_key(), chacha_set_nonce(), chacha_crypt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/chacha.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t key[CHACHA_KEY_SIZE] = {0};
    static const uint8_t nonce[CHACHA_NONCE_SIZE] = {0};
    static const uint8_t zeros[64] = {0};

    /* DJB ChaCha20 keystream for key=0, nonce=0, counter=0, bytes 0..63. */
    static const uint8_t expected_keystream[64] = {
        0x76, 0xb8, 0xe0, 0xad, 0xa0, 0xf1, 0x3d, 0x90,
        0x40, 0x5d, 0x6a, 0xe5, 0x53, 0x86, 0xbd, 0x28,
        0xbd, 0xd2, 0x19, 0xb8, 0xa0, 0x8d, 0xed, 0x1a,
        0xa8, 0x36, 0xef, 0xcc, 0x8b, 0x77, 0x0d, 0xc7,
        0xda, 0x41, 0x59, 0x7c, 0x51, 0x57, 0x48, 0x8d,
        0x77, 0x24, 0xe0, 0x3f, 0xb8, 0xd8, 0x4a, 0x37,
        0x6a, 0x43, 0xb8, 0xf4, 0x15, 0x18, 0xa1, 0x1c,
        0xc3, 0x87, 0xb6, 0x69, 0xb2, 0xee, 0x65, 0x86,
    };

    /* --- Encrypt 64 zero bytes — output IS the keystream. --- */
    struct chacha_ctx ctx;
    uint8_t out[64];
    chacha_set_key(&ctx, key);
    chacha_set_nonce(&ctx, nonce);
    chacha_crypt(&ctx, sizeof(zeros), out, zeros);
    assert(memcmp(out, expected_keystream, sizeof(expected_keystream)) == 0);

    static const uint8_t plaintext[64] = {
        'T', 'h', 'e', ' ', 'q', 'u', 'i', 'c',
        'k', ' ', 'b', 'r', 'o', 'w', 'n', ' ',
        'f', 'o', 'x', ' ', 'j', 'u', 'm', 'p',
        's', ' ', 'o', 'v', 'e', 'r', ' ', 't',
        'h', 'e', ' ', 'l', 'a', 'z', 'y', ' ',
        'd', 'o', 'g', '.', ' ', '0', '1', '2',
        '3', '4', '5', '6', '7', '8', '9', '!',
        '@', '#', '$', '%', '^', '&', '*', '(',
    };
    uint8_t ct[64];
    uint8_t pt_recovered[64];
    struct chacha_ctx enc, dec;
    chacha_set_key(&enc, key);
    chacha_set_nonce(&enc, nonce);
    chacha_crypt(&enc, sizeof(plaintext), ct, plaintext);
    /* ct must differ from plaintext (sanity floor). */
    assert(memcmp(ct, plaintext, sizeof(plaintext)) != 0);

    chacha_set_key(&dec, key);
    chacha_set_nonce(&dec, nonce);
    chacha_crypt(&dec, sizeof(ct), pt_recovered, ct);
    assert(memcmp(pt_recovered, plaintext, sizeof(plaintext)) == 0);

    uint8_t alt_nonce[CHACHA_NONCE_SIZE] = {0};
    alt_nonce[0] = 1;
    struct chacha_ctx alt_ctx;
    uint8_t alt_out[64];
    chacha_set_key(&alt_ctx, key);
    chacha_set_nonce(&alt_ctx, alt_nonce);
    chacha_crypt(&alt_ctx, sizeof(zeros), alt_out, zeros);
    assert(memcmp(alt_out, expected_keystream, sizeof(expected_keystream)) != 0);

    printf("nettle chacha test OK\n");
    return 0;
}
