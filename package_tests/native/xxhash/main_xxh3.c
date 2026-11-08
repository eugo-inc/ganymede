/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xxhash: xxh3; calls XXH3_64bits(), XXH3_createState(), XXH3_64bits_reset(), XXH3_64bits_update().
 */

#include <xxhash.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const char data[] =
        "Demo xxhash XXH3_64bits functional probe — payload long enough "
        "to exercise the >240-byte XXH3 path as well as the small-input "
        "code paths. Pad pad pad pad pad pad pad pad pad pad pad pad pad. "
        "Demo xxhash XXH3_64bits functional probe — second sentence that "
        "takes the payload past 240 bytes and past the 256-byte streaming "
        "buffer, so the streamed digest below crosses a buffer flush.";
    const size_t n = sizeof(data) - 1;
    assert(n > 256); /* above XXH3's 240-byte mid-size limit and its 256-byte stream buffer */

    /* One-shot XXH3_64bits, no seed. */
    XXH64_hash_t one_shot = XXH3_64bits(data, n);

    XXH3_state_t *state = XXH3_createState();
    assert(state != NULL);
    assert(XXH3_64bits_reset(state) == XXH_OK);
    assert(XXH3_64bits_update(state, data, 7) == XXH_OK);
    assert(XXH3_64bits_update(state, data + 7, 33) == XXH_OK);
    assert(XXH3_64bits_update(state, data + 40, n - 40) == XXH_OK);
    XXH64_hash_t streamed = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    assert(streamed == one_shot);

    /* Seed must influence the result. */
    XXH64_hash_t seeded = XXH3_64bits_withSeed(data, n, 0xDEADBEEFCAFEBABEULL);
    assert(seeded != one_shot);

    /* XXH3 is a DIFFERENT algorithm from XXH64 — should diverge on the
     * same input (astronomically unlikely to collide on this payload). */
    XXH64_hash_t xxh64_value = XXH64(data, n, 0);
    assert(xxh64_value != one_shot);

    printf("xxhash XXH3 test OK: %016llx\n", (unsigned long long)one_shot);
    return 0;
}
