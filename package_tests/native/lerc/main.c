/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lerc: core API; calls lerc_computeCompressedSize(), lerc_encode(), lerc_getBlobInfo(), lerc_decode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 8
#define N_ROWS 4
#define N_PIXELS (N_COLS * N_ROWS)

int main(void) {
    float src[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        src[i] = (float)i * 0.5f;
    }

    /* dataType 6 = float; nDepth=1, nBands=1, nMasks=0 (all valid). */
    unsigned int needed = 0;
    lerc_status st = lerc_computeCompressedSize(
        src, 6, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0, &needed);
    assert(st == 0);
    assert(needed > 0);
    assert(needed <= 65536);

    unsigned char blob[65536];
    unsigned int written = 0;
    st = lerc_encode(
        src, 6, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0,
        blob, sizeof(blob), &written);
    assert(st == 0);
    assert(written > 0);
    assert(written <= needed);

    unsigned int info[11] = {0};
    double ranges[3] = {0};
    st = lerc_getBlobInfo(blob, written, info, ranges, 11, 3);
    assert(st == 0);
    assert(info[1] == 6);      /* dataType  */
    assert(info[3] == N_COLS); /* nCols     */
    assert(info[4] == N_ROWS); /* nRows     */
    assert(info[5] == 1);      /* nBands    */

    /* Decode and compare lossless (maxZErr was 0). */
    float dst[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    st = lerc_decode(blob, written, 0, NULL, 1, N_COLS, N_ROWS, 1, 6, dst);
    assert(st == 0);
    for (int i = 0; i < N_PIXELS; ++i) {
        assert(dst[i] == src[i]);
    }

    printf("lerc test OK: encoded %u bytes, round-tripped %d floats\n",
           written, N_PIXELS);
    return 0;
}
