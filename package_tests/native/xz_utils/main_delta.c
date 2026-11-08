/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xz_utils: delta; calls lzma_lzma_preset(), lzma_raw_encoder_memusage(), lzma_raw_buffer_encode(), lzma_raw_buffer_decode().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    /* A monotonically-increasing byte sequence — perfect candidate for
     * Delta(dist=1) preprocessing, which turns it into a stream of mostly
     * identical bytes that LZMA2 then crushes. */
    unsigned char input[1024];
    for (size_t i = 0; i < sizeof(input); ++i)
        input[i] = (unsigned char)(i & 0xFF);
    const size_t input_size = sizeof(input);

    lzma_options_delta delta_opts = { .type = LZMA_DELTA_TYPE_BYTE, .dist = 1 };
    lzma_options_lzma lzma_opts;
    lzma_bool preset_failed = lzma_lzma_preset(&lzma_opts, 6);
    assert(preset_failed == 0);

    lzma_filter filters[3];
    filters[0].id = LZMA_FILTER_DELTA;
    filters[0].options = &delta_opts;
    filters[1].id = LZMA_FILTER_LZMA2;
    filters[1].options = &lzma_opts;
    filters[2].id = LZMA_VLI_UNKNOWN;  /* terminator */
    filters[2].options = NULL;

    /* Sanity: the chain is well-formed. memusage returns UINT64_MAX on error. */
    uint64_t mem = lzma_raw_encoder_memusage(filters);
    assert(mem != UINT64_MAX);
    assert(mem > 0);

    /* ---------- ENCODE ---------- */
    unsigned char compressed[2048];
    size_t out_pos = 0;
    lzma_ret rc = lzma_raw_buffer_encode(
        filters, NULL,
        input, input_size, compressed, &out_pos, sizeof(compressed));
    assert(rc == LZMA_OK);
    assert(out_pos > 0);
    assert(out_pos < input_size);  /* Delta+LZMA2 must shrink monotonic input */

    /* ---------- DECODE ---------- */
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

    /* ---------- VALIDATION ---------- */
    /* Delta dist=0 is below LZMA_DELTA_DIST_MIN (=1) → must fail validation. */
    lzma_options_delta bad_delta = { .type = LZMA_DELTA_TYPE_BYTE, .dist = 0 };
    lzma_filter bad_filters[3];
    bad_filters[0].id = LZMA_FILTER_DELTA;
    bad_filters[0].options = &bad_delta;
    bad_filters[1].id = LZMA_FILTER_LZMA2;
    bad_filters[1].options = &lzma_opts;
    bad_filters[2].id = LZMA_VLI_UNKNOWN;
    bad_filters[2].options = NULL;

    uint64_t bad_mem = lzma_raw_encoder_memusage(bad_filters);
    assert(bad_mem == UINT64_MAX);  /* options-validation must reject */

    printf("xz_utils delta+lzma2 chain test OK: %zu -> %zu bytes\n",
           input_size, out_pos);
    return 0;
}
