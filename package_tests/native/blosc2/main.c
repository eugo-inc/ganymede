/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for blosc2: core API; calls blosc2_init(), blosc2_compress(), blosc2_decompress(), blosc2_destroy().
 */

#include <blosc2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    blosc2_init();

    /* Highly compressible buffer — repeating pattern. typesize=4 matches sizeof(int). */
    enum { N = 1024 };
    int input[N];
    for (int i = 0; i < N; i++) {
        input[i] = i % 16;
    }
    const int input_bytes = (int)sizeof(input);

    /* blosc2 contract: dest buffer must be at least srcsize + BLOSC2_MAX_OVERHEAD. */
    unsigned char compressed[sizeof(input) + BLOSC2_MAX_OVERHEAD];
    int csize = blosc2_compress(
        5, BLOSC_SHUFFLE, sizeof(int),
        input, input_bytes,
        compressed, (int)sizeof(compressed));
    assert(csize > 0);
    assert(csize < input_bytes);

    int decompressed[N];
    int dsize = blosc2_decompress(compressed, csize, decompressed, (int)sizeof(decompressed));
    assert(dsize == input_bytes);
    assert(memcmp(decompressed, input, input_bytes) == 0);

    blosc2_destroy();

    const char *v = blosc2_get_version_string();
    assert(v != NULL);

    printf("blosc2 test OK: %s\n", v);
    return 0;
}
