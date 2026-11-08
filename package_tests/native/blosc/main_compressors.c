/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for blosc: compressors; calls blosc_compress_ctx(), blosc_decompress_ctx(), blosc_list_compressors().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void roundtrip_with(const char *compname) {
    /* Use a payload large enough that every codec actually compresses it. */
    const size_t n = 4096;
    unsigned char *input = (unsigned char *)malloc(n);
    assert(input != NULL);
    for (size_t i = 0; i < n; ++i) {
        input[i] = (unsigned char)(i % 17);  /* compressible repeating pattern */
    }

    const size_t dest_size = n + BLOSC_MAX_OVERHEAD;
    unsigned char *compressed = (unsigned char *)malloc(dest_size);
    unsigned char *decompressed = (unsigned char *)malloc(n);
    assert(compressed != NULL && decompressed != NULL);

    int csize = blosc_compress_ctx(
        /*clevel=*/5, /*doshuffle=*/BLOSC_SHUFFLE,
        /*typesize=*/1, /*nbytes=*/n,
        input, compressed, dest_size,
        compname, /*blocksize=*/0, /*numinternalthreads=*/1);
    assert(csize > 0);

    int dsize = blosc_decompress_ctx(
        compressed, decompressed, n, /*numinternalthreads=*/1);
    assert((size_t)dsize == n);
    assert(memcmp(decompressed, input, n) == 0);

    free(input);
    free(compressed);
    free(decompressed);
}

int main(void) {
    const char *all = blosc_list_compressors();
    assert(all != NULL);
    assert(strstr(all, "blosclz") != NULL);
    assert(strstr(all, "lz4") != NULL);
    assert(strstr(all, "zlib") != NULL);
    assert(strstr(all, "zstd") != NULL);

    roundtrip_with("blosclz");
    roundtrip_with("lz4");
    /* snappy is an optional codec; exercise it only when this build has it. */
    if (strstr(all, "snappy") != NULL) {
        roundtrip_with("snappy");
    }
    roundtrip_with("zlib");
    roundtrip_with("zstd");

    printf("blosc compressor test OK: codecs=%s\n", all);
    return 0;
}
