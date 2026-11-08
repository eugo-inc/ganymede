/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lerc: lossy tolerance; calls lerc_encode(), lerc_decode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 16
#define N_ROWS 16
#define N_PIXELS (N_COLS * N_ROWS)

int main(void) {
    float src[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        /* Mix of values across [0, 1000] so a 0.5 tolerance is meaningful. */
        src[i] = (float)((i * 37 + 11) % 1000) + 0.123f;
    }

    /* dataType 6 = float, single band, no mask. */
    unsigned char blob_lossless[262144];
    unsigned int written_lossless = 0;
    lerc_status st = lerc_encode(
        src, /*dataType=*/6, /*nDepth=*/1, N_COLS, N_ROWS,
        /*nBands=*/1, /*nMasks=*/0, NULL, /*maxZErr=*/0.0,
        blob_lossless, sizeof(blob_lossless), &written_lossless);
    assert(st == 0);
    assert(written_lossless > 0);

    const double max_z_err = 0.5;
    unsigned char blob_lossy[262144];
    unsigned int written_lossy = 0;
    st = lerc_encode(
        src, 6, 1, N_COLS, N_ROWS, 1, 0, NULL, max_z_err,
        blob_lossy, sizeof(blob_lossy), &written_lossy);
    assert(st == 0);
    assert(written_lossy > 0);

    /* Lossy must be strictly smaller — if it weren't, maxZErr was ignored. */
    assert(written_lossy < written_lossless);

    /* Decode the lossy blob and verify every sample stays within tolerance. */
    float dst[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    st = lerc_decode(blob_lossy, written_lossy, 0, NULL, 1, N_COLS, N_ROWS, 1, 6, dst);
    assert(st == 0);

    for (int i = 0; i < N_PIXELS; ++i) {
        double err = fabs((double)dst[i] - (double)src[i]);
        assert(err <= max_z_err);
    }

    printf("lerc lossy/maxZErr OK: lossless=%u bytes, lossy=%u bytes, max err <= %.3f\n",
           written_lossless, written_lossy, max_z_err);
    return 0;
}
