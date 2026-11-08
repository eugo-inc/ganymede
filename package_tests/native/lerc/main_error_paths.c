/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lerc: error paths; calls lerc_encode(), lerc_getBlobInfo(), lerc_decode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 8
#define N_ROWS 8
#define N_PIXELS (N_COLS * N_ROWS)

int main(void) {
    float src[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        src[i] = (float)i * 1.5f + 2.0f;
    }

    /* --- Path 1: encode into an undersized output buffer must FAIL. --- */
    unsigned char tiny_blob[1] = {0};
    unsigned int written_tiny = 0;
    lerc_status st_tiny = lerc_encode(
        src, /*dataType=*/6 /* float */, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0,
        tiny_blob, sizeof(tiny_blob), &written_tiny);
    assert(st_tiny != 0);

    /* --- Path 2: getBlobInfo on random / non-lerc bytes must FAIL. --- */
    unsigned char garbage[64];
    for (int i = 0; i < (int)sizeof(garbage); ++i) {
        garbage[i] = (unsigned char)(i * 73 + 5);
    }
    unsigned int info[11] = {0};
    double ranges[3] = {0};
    lerc_status st_garbage = lerc_getBlobInfo(garbage, sizeof(garbage), info, ranges, 11, 3);
    assert(st_garbage != 0);

    /* --- Path 3: decode of a truncated blob must FAIL. --- */
    unsigned char good_blob[16384];
    unsigned int written_good = 0;
    lerc_status st_good = lerc_encode(
        src, 6, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0,
        good_blob, sizeof(good_blob), &written_good);
    assert(st_good == 0);
    assert(written_good > 16); /* sanity — way more than a header */

    /* Pass only the first half of the blob — the decoder must refuse. */
    unsigned int truncated_len = written_good / 2;
    float dst[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    lerc_status st_trunc = lerc_decode(
        good_blob, truncated_len, 0, NULL, 1, N_COLS, N_ROWS, 1, 6, dst);
    assert(st_trunc != 0);

    printf("lerc error-paths OK: undersized=%d, garbage=%d, truncated=%d\n",
           st_tiny, st_garbage, st_trunc);
    return 0;
}
