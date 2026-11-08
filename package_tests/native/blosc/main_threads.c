/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for blosc: threads; calls blosc_init(), blosc_set_nthreads(), blosc_get_nthreads(), blosc_compress().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc_init();

    /* Start from 1 thread so the "previous" return value is deterministic. */
    int prev_a = blosc_set_nthreads(1);
    assert(prev_a >= 1);
    assert(blosc_get_nthreads() == 1);

    /* Set to 4 and verify the previous (=1) is returned. */
    int prev_b = blosc_set_nthreads(4);
    assert(prev_b == 1);
    assert(blosc_get_nthreads() == 4);

    /* Large-enough payload that the pool actually fans out blocks. */
    const size_t nelems = 64 * 1024;            /* 64K * sizeof(int) = 256 KB */
    const size_t typesize = sizeof(int);
    const size_t nbytes_in = nelems * typesize;

    int *input = (int *)malloc(nbytes_in);
    assert(input != NULL);
    for (size_t i = 0; i < nelems; ++i) {
        input[i] = (int)(i & 0xFFFF);           /* deterministic, compressible */
    }

    const size_t dest_size = nbytes_in + BLOSC_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    unsigned char *decompressed = (unsigned char *)malloc(nbytes_in);
    assert(compressed != NULL && decompressed != NULL);

    int csize = blosc_compress(/*clevel=*/5, BLOSC_SHUFFLE, typesize,
                               nbytes_in, input, compressed, dest_size);
    assert(csize > 0);
    assert((size_t)csize < nbytes_in);          /* must actually compress */

    int dsize = blosc_decompress(compressed, decompressed, nbytes_in);
    assert((size_t)dsize == nbytes_in);
    assert(memcmp(decompressed, input, nbytes_in) == 0);

    int prev_c = blosc_set_nthreads(1);
    assert(prev_c >= 1);
    assert(blosc_get_nthreads() == 1);

    free(input);
    free(compressed);
    free(decompressed);
    blosc_destroy();

    printf("blosc threading test OK: round-trip survived 4-thread pool\n");
    return 0;
}
