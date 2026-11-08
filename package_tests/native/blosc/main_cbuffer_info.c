/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc: cbuffer info; calls blosc_init(), blosc_compress(), blosc_cbuffer_sizes(), blosc_cbuffer_metainfo().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc_init();

    /* 256 int32 values — typesize=4, total = 1024 bytes. */
    const size_t nelems = 256;
    const size_t typesize = sizeof(int);
    const size_t nbytes_in = nelems * typesize;

    int *input = (int *)malloc(nbytes_in);
    assert(input != NULL);
    for (size_t i = 0; i < nelems; ++i) {
        input[i] = (int)(i * 3);  /* deterministic, compressible */
    }

    const size_t dest_size = nbytes_in + BLOSC_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    assert(compressed != NULL);

    int csize = blosc_compress(/*clevel=*/5, BLOSC_SHUFFLE, typesize,
                               nbytes_in, input, compressed, dest_size);
    assert(csize > 0);

    /* --- blosc_cbuffer_sizes: payload accounting --- */
    size_t header_nbytes = 0, header_cbytes = 0, header_blocksize = 0;
    blosc_cbuffer_sizes(compressed, &header_nbytes, &header_cbytes,
                        &header_blocksize);
    assert(header_nbytes == nbytes_in);
    assert(header_cbytes == (size_t)csize);
    assert(header_blocksize > 0);

    /* --- blosc_cbuffer_metainfo: typesize + flags --- */
    size_t header_typesize = 0;
    int header_flags = 0;
    blosc_cbuffer_metainfo(compressed, &header_typesize, &header_flags);
    assert(header_typesize == typesize);
    assert((header_flags & BLOSC_DOSHUFFLE) != 0);

    /* --- blosc_cbuffer_complib: codec library identification --- */
    const char *complib = blosc_cbuffer_complib(compressed);
    assert(complib != NULL);
    assert(strlen(complib) > 0);

    free(input);
    free(compressed);
    blosc_destroy();

    printf("blosc cbuffer-info test OK: complib=%s blocksize=%zu\n",
           complib, header_blocksize);
    return 0;
}
