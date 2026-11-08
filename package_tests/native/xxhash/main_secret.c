/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xxhash: secret; calls XXH3_generateSecret_fromSeed(), XXH3_64bits_withSeed(), XXH3_64bits_withSecret(), XXH3_128bits_withSeed().
 */

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    static const char data[] =
        "Demo XXH3 custom-secret probe payload — long enough to exercise "
        "the >240-byte path through the secret-driven mixer. "
        "Demo XXH3 custom-secret probe payload — long enough to exercise "
        "the >240-byte path through the secret-driven mixer. "
        "Demo XXH3 custom-secret probe payload — long enough to exercise "
        "the >240-byte path through the secret-driven mixer.";
    const size_t n = sizeof(data) - 1;
    /* XXH3's mid-size path ends at 240 bytes; the identities below are the
     * documented long-input ones, so the payload must stay above that. */
    assert(n > 240);

    XXH64_hash_t seed = 0xA5A5A5A5A5A5A5A5ULL;
    unsigned char custom_secret[XXH3_SECRET_DEFAULT_SIZE];
    XXH3_generateSecret_fromSeed(custom_secret, seed);

    /* Sanity: the generated secret is NOT all-zero (catches a fill bug). */
    int nonzero = 0;
    for (size_t i = 0; i < XXH3_SECRET_DEFAULT_SIZE; ++i) {
        if (custom_secret[i] != 0) { nonzero = 1; break; }
    }
    assert(nonzero);

    /* Contract: withSecret(generateSecret_fromSeed(s)) == withSeed(s). */
    XXH64_hash_t via_seed     = XXH3_64bits_withSeed(data, n, seed);
    XXH64_hash_t via_secret64 = XXH3_64bits_withSecret(
        data, n, custom_secret, XXH3_SECRET_DEFAULT_SIZE);
    assert(via_seed == via_secret64);

    /* For the 128-bit family, use the _withSecretandSeed variant: when the
     * secret was produced by generateSecret_fromSeed(s),
     * withSecretandSeed(..., s) reproduces withSeed(s) *exactly* at every
     * input length. */
    XXH128_hash_t via_seed128 = XXH3_128bits_withSeed(data, n, seed);
    XXH128_hash_t via_secret128 = XXH3_128bits_withSecretandSeed(
        data, n, custom_secret, XXH3_SECRET_DEFAULT_SIZE, seed);
    assert(XXH128_isEqual(via_seed128, via_secret128));

    XXH128_hash_t bare_secret128 = XXH3_128bits_withSecret(
        data, n, custom_secret, XXH3_SECRET_DEFAULT_SIZE);
    assert(bare_secret128.low64 != 0 || bare_secret128.high64 != 0);

    XXH3_state_t *state = XXH3_createState();
    assert(state != NULL);
    assert(XXH3_64bits_reset_withSecret(
        state, custom_secret, XXH3_SECRET_DEFAULT_SIZE) == XXH_OK);
    assert(XXH3_64bits_update(state, data, 13) == XXH_OK);
    assert(XXH3_64bits_update(state, data + 13, n - 13) == XXH_OK);
    XXH64_hash_t streamed_secret = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    assert(streamed_secret == via_secret64);

    /* A different secret must produce a different digest. */
    unsigned char other_secret[XXH3_SECRET_DEFAULT_SIZE];
    XXH3_generateSecret_fromSeed(other_secret, seed ^ 0xFFULL);
    XXH64_hash_t other_digest = XXH3_64bits_withSecret(
        data, n, other_secret, XXH3_SECRET_DEFAULT_SIZE);
    assert(other_digest != via_secret64);

    printf("xxhash custom-secret test OK: %016llx\n",
           (unsigned long long)via_secret64);
    return 0;
}
