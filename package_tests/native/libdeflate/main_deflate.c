/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libdeflate: deflate; calls libdeflate_alloc_compressor(), libdeflate_deflate_compress_bound(), libdeflate_deflate_compress(), libdeflate_alloc_decompressor().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libdeflate.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *input =
        "Sphinx of black quartz, judge my vow. "
        "Sphinx of black quartz, judge my vow. "
        "Sphinx of black quartz, judge my vow.";
    const size_t input_len = strlen(input);

    struct libdeflate_compressor *c = libdeflate_alloc_compressor(9);
    assert(c != NULL);

    unsigned char compressed[512];
    size_t bound = libdeflate_deflate_compress_bound(c, input_len);
    assert(bound > 0);
    assert(bound <= sizeof(compressed));

    size_t compressed_len = libdeflate_deflate_compress(
        c, input, input_len, compressed, sizeof(compressed));
    assert(compressed_len > 0);
    /* Repetitive input must compress smaller than the source. */
    assert(compressed_len < input_len);

    struct libdeflate_decompressor *d = libdeflate_alloc_decompressor();
    assert(d != NULL);

    unsigned char decompressed[512] = {0};
    size_t decompressed_len = 0;
    enum libdeflate_result rc = libdeflate_deflate_decompress(
        d, compressed, compressed_len,
        decompressed, sizeof(decompressed), &decompressed_len);
    assert(rc == LIBDEFLATE_SUCCESS);
    assert(decompressed_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    libdeflate_free_decompressor(d);
    libdeflate_free_compressor(c);

    printf("libdeflate raw-deflate test OK\n");
    return 0;
}
