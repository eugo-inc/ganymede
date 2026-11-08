/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xz_utils: raw; calls lzma_lzma_preset(), lzma_raw_buffer_encode(), lzma_raw_buffer_decode().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    static const unsigned char input[] =
        "raw filter test payload: "
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
    const size_t input_size = sizeof(input) - 1;

    lzma_options_lzma opts;
    lzma_bool preset_failed = lzma_lzma_preset(&opts, 6);
    assert(preset_failed == 0);  /* 0 = success, 1 = unknown preset */

    lzma_filter filters[2];
    filters[0].id = LZMA_FILTER_LZMA2;
    filters[0].options = &opts;
    filters[1].id = LZMA_VLI_UNKNOWN;  /* sentinel terminating the chain */
    filters[1].options = NULL;

    /* ---------- RAW ENCODE ---------- */
    unsigned char compressed[1024];
    size_t out_pos = 0;
    lzma_ret rc = lzma_raw_buffer_encode(
        filters, NULL,
        input, input_size, compressed, &out_pos, sizeof(compressed));
    assert(rc == LZMA_OK);
    assert(out_pos > 0);
    assert(out_pos < input_size);  /* highly redundant input -> must shrink */

    /* ---------- RAW DECODE ---------- */
    unsigned char decompressed[sizeof(input)];
    size_t in_pos = 0;
    size_t dec_pos = 0;
    rc = lzma_raw_buffer_decode(
        filters, NULL,
        compressed, &in_pos, out_pos,
        decompressed, &dec_pos, sizeof(decompressed));
    assert(rc == LZMA_OK);
    assert(in_pos == out_pos);
    assert(dec_pos == input_size);
    assert(memcmp(decompressed, input, input_size) == 0);

    printf("xz_utils raw-filter test OK: %zu -> %zu bytes (LZMA2 preset 6)\n",
           input_size, out_pos);
    return 0;
}
