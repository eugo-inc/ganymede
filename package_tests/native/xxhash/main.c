/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xxhash: core API; calls XXH64_createState(), XXH64_reset(), XXH64_update(), XXH64_digest().
 */

#include <xxhash.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(XXH32(NULL, 0, 0) == 0x02CC5D05U);
    assert(XXH64(NULL, 0, 0) == 0xEF46DB3751D8E999ULL);

    static const char data[] = "The quick brown fox jumps over the lazy dog";
    const size_t n = sizeof(data) - 1;

    XXH64_hash_t one_shot = XXH64(data, n, 0);

    XXH64_state_t *state = XXH64_createState();
    assert(state != NULL);
    assert(XXH64_reset(state, 0) == XXH_OK);
    /* Feed in two chunks to exercise the buffered path. */
    assert(XXH64_update(state, data, 10) == XXH_OK);
    assert(XXH64_update(state, data + 10, n - 10) == XXH_OK);
    XXH64_hash_t streamed = XXH64_digest(state);
    XXH64_freeState(state);

    assert(streamed == one_shot);

    printf("xxhash test OK: %u.%u.%u\n",
           XXH_VERSION_MAJOR, XXH_VERSION_MINOR, XXH_VERSION_RELEASE);
    return 0;
}
