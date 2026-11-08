/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for blosc2: cbuffer info; calls blosc2_init(), blosc2_compress(), blosc2_cbuffer_sizes(), blosc2_cbuffer_versions().
 */

#include <blosc2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc2_init();

    /* 256 int32 values — typesize=4, total = 1024 bytes. */
    enum { NELEMS = 256 };
    const int32_t typesize = (int32_t)sizeof(int);
    const int32_t nbytes_in = NELEMS * typesize;

    int input[NELEMS];
    for (int i = 0; i < NELEMS; i++) {
        input[i] = i * 3;  /* deterministic, compressible */
    }

    const int32_t dest_size = nbytes_in + BLOSC2_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc((size_t)dest_size);
    assert(compressed != NULL);

    int csize = blosc2_compress(/*clevel=*/5, BLOSC_SHUFFLE, typesize,
                                input, nbytes_in, compressed, dest_size);
    assert(csize > 0);

    /* --- blosc2_cbuffer_sizes: payload accounting --- */
    int32_t header_nbytes = 0, header_cbytes = 0, header_blocksize = 0;
    int rc = blosc2_cbuffer_sizes(compressed, &header_nbytes,
                                  &header_cbytes, &header_blocksize);
    assert(rc >= 0);
    assert(header_nbytes == nbytes_in);
    assert(header_cbytes == csize);
    assert(header_blocksize > 0);

    /* --- blosc2_cbuffer_versions: format-version + lz-version --- */
    int version = 0, versionlz = 0;
    blosc2_cbuffer_versions(compressed, &version, &versionlz);
    /* Blosc2 chunks use format version >= 2. */
    assert(version >= 2);
    assert(versionlz > 0);

    /* --- blosc2_cbuffer_complib: codec library identification --- */
    const char *complib = blosc2_cbuffer_complib(compressed);
    assert(complib != NULL);
    assert(strlen(complib) > 0);

    free(compressed);
    blosc2_destroy();

    printf("blosc2 cbuffer-info test OK: complib=%s blocksize=%d version=%d\n",
           complib, header_blocksize, version);
    return 0;
}
