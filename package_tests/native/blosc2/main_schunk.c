/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc2: schunk; calls blosc2_init(), blosc2_schunk_new(), blosc2_schunk_append_buffer(), blosc2_schunk_decompress_chunk().
 */

#include <blosc2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc2_init();

    /* In-memory super-chunk with default cparams/dparams. */
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.typesize = (int32_t)sizeof(int);
    cparams.clevel = 5;
    cparams.nthreads = 1;

    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 1;

    blosc2_storage storage = {
        .contiguous = false,
        .urlpath = NULL,
        .cparams = &cparams,
        .dparams = &dparams,
        .io = NULL,
    };

    blosc2_schunk *schunk = blosc2_schunk_new(&storage);
    assert(schunk != NULL);
    assert(schunk->nchunks == 0);

    /* Append 3 chunks of 1024 ints (4096 bytes) each, with distinct patterns. */
    enum { NCHUNKS = 3, N = 1024 };
    const int32_t chunk_bytes = (int32_t)(N * sizeof(int));
    int chunks[NCHUNKS][N];
    for (int c = 0; c < NCHUNKS; c++) {
        for (int i = 0; i < N; i++) {
            chunks[c][i] = (c * 1000) + (i % 16);
        }
        int64_t nappended = blosc2_schunk_append_buffer(
            schunk, chunks[c], chunk_bytes);
        assert(nappended == (int64_t)(c + 1));
    }

    assert(schunk->nchunks == NCHUNKS);
    /* nbytes is the cumulative uncompressed payload. */
    assert(schunk->nbytes == (int64_t)NCHUNKS * chunk_bytes);
    /* cbytes is the cumulative compressed payload; smaller for compressible input. */
    assert(schunk->cbytes > 0);
    assert(schunk->cbytes < schunk->nbytes);

    int decoded[N];
    for (int c = 0; c < NCHUNKS; c++) {
        int dsize = blosc2_schunk_decompress_chunk(
            schunk, (int64_t)c, decoded, chunk_bytes);
        assert(dsize == chunk_bytes);
        assert(memcmp(decoded, chunks[c], (size_t)chunk_bytes) == 0);
    }

    const int64_t total_uncompressed = schunk->nbytes;
    const int64_t total_compressed = schunk->cbytes;

    int rc = blosc2_schunk_free(schunk);
    assert(rc == 0);

    blosc2_destroy();

    printf("blosc2 schunk test OK: nchunks=%d ratio=%.2fx\n",
           NCHUNKS,
           (double)total_uncompressed / (double)total_compressed);
    return 0;
}
