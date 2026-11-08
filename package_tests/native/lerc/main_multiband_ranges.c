/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lerc: multiband ranges; calls lerc_computeCompressedSize(), lerc_encode(), lerc_getBlobInfo(), lerc_getDataRanges().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 4
#define N_ROWS 3
#define N_BANDS 3
#define N_DEPTH 2
#define N_VALUES_PER_BAND (N_COLS * N_ROWS * N_DEPTH)
#define N_TOTAL (N_VALUES_PER_BAND * N_BANDS)

int main(void) {
    int src[N_TOTAL];
    int expected_min[N_BANDS * N_DEPTH];
    int expected_max[N_BANDS * N_DEPTH];
    for (int i = 0; i < N_BANDS * N_DEPTH; ++i) {
        expected_min[i] = 0x7fffffff;
        expected_max[i] = -0x7fffffff - 1;
    }

    for (int b = 0; b < N_BANDS; ++b) {
        for (int r = 0; r < N_ROWS; ++r) {
            for (int c = 0; c < N_COLS; ++c) {
                for (int d = 0; d < N_DEPTH; ++d) {
                    int idx = ((b * N_ROWS + r) * N_COLS + c) * N_DEPTH + d;
                    /* Distinct value per (band, depth) channel so ranges differ. */
                    int v = (b * 1000) + (d * 100) + (r * N_COLS + c);
                    src[idx] = v;
                    int chan = b * N_DEPTH + d;
                    if (v < expected_min[chan]) expected_min[chan] = v;
                    if (v > expected_max[chan]) expected_max[chan] = v;
                }
            }
        }
    }

    unsigned int needed = 0;
    lerc_status st = lerc_computeCompressedSize(
        src, /*dataType=*/4 /* int32 */, N_DEPTH, N_COLS, N_ROWS,
        N_BANDS, /*nMasks=*/0, NULL, /*maxZErr=*/0.0, &needed);
    assert(st == 0);
    assert(needed > 0);

    unsigned char blob[131072];
    assert(needed <= sizeof(blob));
    unsigned int written = 0;
    st = lerc_encode(
        src, 4, N_DEPTH, N_COLS, N_ROWS, N_BANDS, 0, NULL, 0.0,
        blob, sizeof(blob), &written);
    assert(st == 0);
    assert(written > 0);

    /* Sanity: getBlobInfo must agree on shape. */
    unsigned int info[11] = {0};
    double overall_range[3] = {0};
    st = lerc_getBlobInfo(blob, written, info, overall_range, 11, 3);
    assert(st == 0);
    assert(info[5] == (unsigned)N_BANDS);

    /* lerc_getDataRanges populates per-(depth, band) min/max. The output buffers
     * must be (nDepth * nBands) doubles. */
    double mins[N_DEPTH * N_BANDS];
    double maxs[N_DEPTH * N_BANDS];
    for (int i = 0; i < N_DEPTH * N_BANDS; ++i) {
        mins[i] = 1.0;
        maxs[i] = -1.0;
    }
    st = lerc_getDataRanges(blob, written, N_DEPTH, N_BANDS, mins, maxs);
    assert(st == 0);

    for (int chan = 0; chan < N_BANDS * N_DEPTH; ++chan) {
        assert(mins[chan] == (double)expected_min[chan]);
        assert(maxs[chan] == (double)expected_max[chan]);
    }

    printf("lerc multi-band/getDataRanges OK: %u bytes, %d channels verified\n",
           written, N_BANDS * N_DEPTH);
    return 0;
}
