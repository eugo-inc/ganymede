/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xxhash: xxh128; calls XXH128_isEqual(), XXH3_createState(), XXH3_128bits_reset(), XXH3_128bits_update().
 */

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const char data[] =
        "Demo XXH128 functional probe payload — long enough to land in the "
        "large-input XXH3 path. Different content from the XXH3_64bits probe. "
        "Demo XXH128 functional probe payload — long enough to land in the "
        "large-input XXH3 path. Different content from the XXH3_64bits probe.";
    const size_t n = sizeof(data) - 1;
    assert(n > 240); /* XXH3's large-input path starts above 240 bytes */

    /* One-shot 128-bit hash. */
    XXH128_hash_t h1 = XXH128(data, n, 0);
    XXH128_hash_t h2 = XXH128(data, n, 0);

    /* Equality helper agrees with manual field compare. */
    assert(XXH128_isEqual(h1, h2));
    assert(h1.low64 == h2.low64);
    assert(h1.high64 == h2.high64);

    assert(h1.low64 != 0);
    assert(h1.high64 != 0);

    /* Different seed must produce a different hash. */
    XXH128_hash_t h_seeded = XXH128(data, n, 0x123456789ABCDEF0ULL);
    assert(!XXH128_isEqual(h1, h_seeded));

    XXH3_state_t *state = XXH3_createState();
    assert(state != NULL);
    assert(XXH3_128bits_reset(state) == XXH_OK);
    assert(XXH3_128bits_update(state, data, 11) == XXH_OK);
    assert(XXH3_128bits_update(state, data + 11, n - 11) == XXH_OK);
    XXH128_hash_t streamed = XXH3_128bits_digest(state);
    XXH3_freeState(state);
    assert(XXH128_isEqual(streamed, h1));

    printf("xxhash XXH128 test OK: %016llx%016llx\n",
           (unsigned long long)h1.high64,
           (unsigned long long)h1.low64);
    return 0;
}
