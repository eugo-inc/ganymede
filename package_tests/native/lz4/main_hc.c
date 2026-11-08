/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lz4: hc; calls LZ4_compressBound(), LZ4_compress_HC(), LZ4_decompress_safe().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lz4.h>
#include <lz4hc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Highly redundant input — HC should compress well. */
    char input[4096];
    for (int i = 0; i < (int)sizeof(input); ++i) {
        input[i] = (char)('a' + (i % 8));
    }
    const int input_len = (int)sizeof(input);

    const int bound = LZ4_compressBound(input_len);
    assert(bound > 0);

    char compressed_default[8192];
    char compressed_max[8192];
    assert(bound <= (int)sizeof(compressed_default));

    const int n_default = LZ4_compress_HC(input, compressed_default, input_len,
                                          (int)sizeof(compressed_default),
                                          LZ4HC_CLEVEL_DEFAULT);
    assert(n_default > 0);
    assert(n_default < input_len); /* Redundant input MUST shrink. */

    const int n_max = LZ4_compress_HC(input, compressed_max, input_len,
                                      (int)sizeof(compressed_max),
                                      LZ4HC_CLEVEL_MAX);
    assert(n_max > 0);
    /* Higher level should be no worse than default on this input. */
    assert(n_max <= n_default);

    char round_default[4096] = {0};
    char round_max[4096] = {0};
    const int d1 = LZ4_decompress_safe(compressed_default, round_default,
                                       n_default, (int)sizeof(round_default));
    assert(d1 == input_len);
    assert(memcmp(round_default, input, input_len) == 0);

    const int d2 = LZ4_decompress_safe(compressed_max, round_max,
                                       n_max, (int)sizeof(round_max));
    assert(d2 == input_len);
    assert(memcmp(round_max, input, input_len) == 0);

    printf("lz4 HC test OK: default=%d max=%d (input=%d)\n",
           n_default, n_max, input_len);
    return 0;
}
