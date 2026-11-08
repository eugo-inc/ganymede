/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc: core API; calls blosc_init(), blosc_compress(), blosc_decompress(), blosc_destroy().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc_init();

    const char *input = "the quick brown fox jumps over the lazy dog";
    const size_t input_len = strlen(input) + 1;  /* include NUL for strict equality check */
    const size_t typesize = 1;
    const size_t dest_size = input_len + BLOSC_MAX_OVERHEAD;

    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    assert(compressed != NULL);

    /* clevel=5, doshuffle=BLOSC_SHUFFLE, default compressor (blosclz). */
    int csize = blosc_compress(5, BLOSC_SHUFFLE, typesize,
                               input_len, input, compressed, dest_size);
    assert(csize > 0);

    unsigned char *decompressed = (unsigned char *)malloc(input_len);
    assert(decompressed != NULL);

    int dsize = blosc_decompress(compressed, decompressed, input_len);
    assert((size_t)dsize == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    blosc_destroy();

    printf("blosc test OK: %s\n", blosc_get_version_string());
    return 0;
}
