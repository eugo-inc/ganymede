/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liblzf: core API; calls lzf_compress(), lzf_decompress().
 */

#include <lzf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* A string repetitive enough that LZF actually finds matches to compress. */
    const char input[] =
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog.";
    const unsigned int input_len = (unsigned int)(sizeof(input) - 1);

    /* LZF documents that out_len must be > in_len for the worst case; give it some slack. */
    unsigned char compressed[512];
    unsigned int compressed_len =
        lzf_compress(input, input_len, compressed, (unsigned int)sizeof(compressed));
    /* Non-zero return means compression succeeded AND fit in the destination buffer. */
    assert(compressed_len != 0);
    assert(compressed_len < input_len);  /* Repetitive input must actually shrink. */

    unsigned char decompressed[512] = {0};
    unsigned int decompressed_len = lzf_decompress(
        compressed, compressed_len, decompressed, (unsigned int)sizeof(decompressed));
    assert(decompressed_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    printf("liblzf test OK\n");
    return 0;
}
