/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc: bitshuffle; calls blosc_init(), blosc_compress(), blosc_cbuffer_metainfo(), blosc_decompress().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc_init();

    /* 1024 doubles — typesize=8, total = 8192 bytes. Doubles + bitshuffle is
     * the canonical case where the bit-level filter outperforms byte-level
     * (high-bit mantissa redundancy). */
    const size_t nelems = 1024;
    const size_t typesize = sizeof(double);
    const size_t nbytes_in = nelems * typesize;

    double *input = (double *)malloc(nbytes_in);
    assert(input != NULL);
    for (size_t i = 0; i < nelems; ++i) {
        /* Slowly varying values — exposes bitshuffle's mantissa-alignment win. */
        input[i] = (double)i * 0.125;
    }

    const size_t dest_size = nbytes_in + BLOSC_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    double *decompressed = (double *)malloc(nbytes_in);
    assert(compressed != NULL && decompressed != NULL);

    int csize = blosc_compress(/*clevel=*/5, BLOSC_BITSHUFFLE, typesize,
                               nbytes_in, input, compressed, dest_size);
    assert(csize > 0);

    /* Header MUST reflect BITSHUFFLE, NOT regular SHUFFLE. */
    size_t header_typesize = 0;
    int header_flags = 0;
    blosc_cbuffer_metainfo(compressed, &header_typesize, &header_flags);
    assert(header_typesize == typesize);
    assert((header_flags & BLOSC_DOBITSHUFFLE) != 0);
    /* The byte-shuffle bit must NOT be set for a bitshuffled buffer. */
    assert((header_flags & BLOSC_DOSHUFFLE) == 0);

    int dsize = blosc_decompress(compressed, decompressed, nbytes_in);
    assert((size_t)dsize == nbytes_in);
    assert(memcmp(decompressed, input, nbytes_in) == 0);

    free(input);
    free(compressed);
    free(decompressed);
    blosc_destroy();

    printf("blosc bitshuffle test OK: flags=0x%x\n", header_flags);
    return 0;
}
