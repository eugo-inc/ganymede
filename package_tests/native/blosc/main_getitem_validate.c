/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc: getitem validate; calls blosc_init(), blosc_compress(), blosc_cbuffer_validate(), blosc_getitem().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    blosc_init();

    /* 512 int32 — typesize=4, total=2048 bytes. Wide enough that getitem
     * picks up a non-trivial sub-range. */
    const size_t nelems = 512;
    const size_t typesize = sizeof(int);
    const size_t nbytes_in = nelems * typesize;

    int *input = (int *)malloc(nbytes_in);
    assert(input != NULL);
    for (size_t i = 0; i < nelems; ++i) {
        input[i] = (int)(i * 7 + 11);  /* deterministic */
    }

    const size_t dest_size = nbytes_in + BLOSC_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    assert(compressed != NULL);

    int csize = blosc_compress(/*clevel=*/5, BLOSC_SHUFFLE, typesize,
                               nbytes_in, input, compressed, dest_size);
    assert(csize > 0);

    /* --- blosc_cbuffer_validate: structural sanity --- */
    size_t validated_nbytes = 0;
    int v_rc = blosc_cbuffer_validate(compressed, (size_t)csize,
                                      &validated_nbytes);
    assert(v_rc == 0);
    assert(validated_nbytes == nbytes_in);

    /* A truncated buffer MUST be rejected (corrupted-input safety). */
    if ((size_t)csize > BLOSC_MIN_HEADER_LENGTH) {
        size_t bogus_nbytes = 0;
        int v_bad = blosc_cbuffer_validate(compressed,
                                           (size_t)(csize / 2),
                                           &bogus_nbytes);
        assert(v_bad == -1);
    }

    /* --- blosc_getitem: pull a 16-element slice from offset 100 --- */
    const int start = 100;
    const int nitems = 16;
    int *slice = (int *)malloc(nitems * typesize);
    assert(slice != NULL);

    int g_rc = blosc_getitem(compressed, start, nitems, slice);
    assert(g_rc == nitems * (int)typesize);
    for (int i = 0; i < nitems; ++i) {
        assert(slice[i] == input[start + i]);
    }

    free(input);
    free(compressed);
    free(slice);
    blosc_destroy();

    printf("blosc getitem/validate test OK: validated=%zu bytes\n",
           validated_nbytes);
    return 0;
}
