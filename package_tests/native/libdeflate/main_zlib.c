/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libdeflate: zlib; calls libdeflate_alloc_compressor(), libdeflate_zlib_compress_bound(), libdeflate_zlib_compress(), libdeflate_alloc_decompressor().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libdeflate.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *input = "libdeflate zlib wrapper round-trip probe";
    const size_t input_len = strlen(input);

    struct libdeflate_compressor *c = libdeflate_alloc_compressor(3);
    assert(c != NULL);

    unsigned char compressed[256];
    size_t bound = libdeflate_zlib_compress_bound(c, input_len);
    assert(bound > 0);
    assert(bound <= sizeof(compressed));

    size_t compressed_len = libdeflate_zlib_compress(
        c, input, input_len, compressed, sizeof(compressed));
    assert(compressed_len > 0);
    /* zlib stream has at least 2-byte header + 4-byte Adler-32 trailer. */
    assert(compressed_len >= 6);
    /* RFC 1950: low nibble of CMF == 8 means DEFLATE compression method. */
    assert((compressed[0] & 0x0F) == 0x08);

    struct libdeflate_decompressor *d = libdeflate_alloc_decompressor();
    assert(d != NULL);

    unsigned char decompressed[256] = {0};
    size_t decompressed_len = 0;
    enum libdeflate_result rc = libdeflate_zlib_decompress(
        d, compressed, compressed_len,
        decompressed, sizeof(decompressed) - 1, &decompressed_len);
    assert(rc == LIBDEFLATE_SUCCESS);
    assert(decompressed_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    libdeflate_free_decompressor(d);
    libdeflate_free_compressor(c);

    printf("libdeflate zlib test OK\n");
    return 0;
}
