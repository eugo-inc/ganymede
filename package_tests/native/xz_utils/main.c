/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xz_utils: core API; calls lzma_easy_buffer_encode(), lzma_stream_buffer_decode(), lzma_version_string().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char input[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const size_t input_size = sizeof(input) - 1;

    unsigned char compressed[512];
    size_t out_pos = 0;
    lzma_ret rc = lzma_easy_buffer_encode(
        6, LZMA_CHECK_CRC64, NULL,
        input, input_size,
        compressed, &out_pos, sizeof(compressed));
    assert(rc == LZMA_OK);
    assert(out_pos > 0);

    unsigned char decompressed[sizeof(input)];
    size_t in_pos = 0;
    size_t dec_pos = 0;
    uint64_t memlimit = UINT64_MAX;
    rc = lzma_stream_buffer_decode(
        &memlimit, 0, NULL,
        compressed, &in_pos, out_pos,
        decompressed, &dec_pos, sizeof(decompressed));
    assert(rc == LZMA_OK);
    assert(dec_pos == input_size);
    assert(memcmp(decompressed, input, input_size) == 0);

    printf("xz_utils test OK: %s\n", lzma_version_string());
    return 0;
}
