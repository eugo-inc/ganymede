/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc2: compressors; calls blosc2_create_cctx(), blosc2_compress_ctx(), blosc2_free_ctx(), blosc2_create_dctx().
 */

#include <blosc2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void roundtrip_with(uint8_t compcode, const char *compname) {
    /* 4 KB compressible repeating pattern — every codec should shrink it. */
    enum { N = 4096 };
    unsigned char input[N];
    for (int i = 0; i < N; i++) {
        input[i] = (unsigned char)(i % 17);
    }

    const int32_t dest_size = N + BLOSC2_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc((size_t)dest_size);
    unsigned char *decompressed = (unsigned char *)malloc(N);
    assert(compressed != NULL && decompressed != NULL);

    /* Per-codec compression context: clevel=5, shuffle filter, typesize=1. */
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.compcode = compcode;
    cparams.clevel = 5;
    cparams.typesize = 1;
    cparams.nthreads = 1;
    cparams.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_context *cctx = blosc2_create_cctx(cparams);
    assert(cctx != NULL);

    int csize = blosc2_compress_ctx(cctx, input, (int32_t)N,
                                    compressed, dest_size);
    assert(csize > 0);
    assert(csize < (int)N);
    blosc2_free_ctx(cctx);

    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.nthreads = 1;
    blosc2_context *dctx = blosc2_create_dctx(dparams);
    assert(dctx != NULL);

    int dsize = blosc2_decompress_ctx(dctx, compressed, csize,
                                      decompressed, (int32_t)N);
    assert(dsize == (int)N);
    assert(memcmp(decompressed, input, N) == 0);
    blosc2_free_ctx(dctx);

    /* The string-to-code lookup must agree with the numeric constant. */
    int looked_up = blosc2_compname_to_compcode(compname);
    assert(looked_up == (int)compcode);

    free(compressed);
    free(decompressed);
}

int main(void) {
    blosc2_init();

    const char *all = blosc2_list_compressors();
    assert(all != NULL);
    assert(strstr(all, BLOSC_BLOSCLZ_COMPNAME) != NULL);
    assert(strstr(all, BLOSC_LZ4_COMPNAME) != NULL);
    assert(strstr(all, BLOSC_ZLIB_COMPNAME) != NULL);
    assert(strstr(all, BLOSC_ZSTD_COMPNAME) != NULL);

    roundtrip_with(BLOSC_BLOSCLZ, BLOSC_BLOSCLZ_COMPNAME);
    roundtrip_with(BLOSC_LZ4, BLOSC_LZ4_COMPNAME);
    roundtrip_with(BLOSC_LZ4HC, BLOSC_LZ4HC_COMPNAME);
    roundtrip_with(BLOSC_ZLIB, BLOSC_ZLIB_COMPNAME);
    roundtrip_with(BLOSC_ZSTD, BLOSC_ZSTD_COMPNAME);

    blosc2_destroy();

    printf("blosc2 compressors test OK: codecs=%s\n", all);
    return 0;
}
