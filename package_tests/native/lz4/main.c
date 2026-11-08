/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lz4: core API; calls LZ4_compressBound(), LZ4_compress_default(), LZ4_decompress_safe(), LZ4_versionString().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lz4.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *input = "the quick brown fox jumps over the lazy dog";
    const int input_len = (int)strlen(input);

    /* LZ4_compressBound is the documented worst-case output size. */
    char compressed[256];
    const int bound = LZ4_compressBound(input_len);
    assert(bound > 0);
    assert(bound <= (int)sizeof(compressed));

    const int compressed_len = LZ4_compress_default(input, compressed, input_len, sizeof(compressed));
    assert(compressed_len > 0);

    char decompressed[256] = {0};
    const int decompressed_len = LZ4_decompress_safe(compressed, decompressed, compressed_len, sizeof(decompressed) - 1);
    assert(decompressed_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    /* Version string must be non-NULL and non-empty (library contract). */
    const char *v = LZ4_versionString();
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("lz4 test OK: %s\n", v);
    return 0;
}
