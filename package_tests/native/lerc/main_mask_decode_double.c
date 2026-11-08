/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lerc: mask decode double; calls lerc_computeCompressedSize(), lerc_encode(), lerc_getBlobInfo(), lerc_decodeToDouble().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 6
#define N_ROWS 5
#define N_PIXELS (N_COLS * N_ROWS)

int main(void) {
    unsigned char src[N_PIXELS];
    unsigned char valid[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        src[i] = (unsigned char)((i * 7 + 3) & 0xff);
        valid[i] = (i % 4 == 3) ? 0 : 1;
    }

    unsigned int needed = 0;
    lerc_status st = lerc_computeCompressedSize(
        src, /*dataType=*/1, /*nDepth=*/1, N_COLS, N_ROWS,
        /*nBands=*/1, /*nMasks=*/1, valid, /*maxZErr=*/0.0, &needed);
    assert(st == 0);
    assert(needed > 0);
    assert(needed <= 65536);

    unsigned char blob[65536];
    unsigned int written = 0;
    st = lerc_encode(
        src, 1, 1, N_COLS, N_ROWS, 1, 1, valid, 0.0,
        blob, sizeof(blob), &written);
    assert(st == 0);
    assert(written > 0);
    assert(written <= needed);

    /* getBlobInfo must report a nonzero nMasks for the masked encode. */
    unsigned int info[11] = {0};
    double ranges[3] = {0};
    st = lerc_getBlobInfo(blob, written, info, ranges, 11, 3);
    assert(st == 0);
    assert(info[1] == 1);            /* dataType == uchar */
    assert(info[3] == N_COLS);
    assert(info[4] == N_ROWS);
    assert(info[5] == 1);            /* nBands */
    assert(info[6] < (unsigned)N_PIXELS); /* nValidPixels < total (some were masked off) */

    /* Decode via lerc_decodeToDouble — different symbol than main.c uses. */
    double dst[N_PIXELS];
    unsigned char dst_valid[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    memset(dst_valid, 0xff, sizeof(dst_valid));
    st = lerc_decodeToDouble(
        blob, written, /*nMasks=*/1, dst_valid,
        /*nDepth=*/1, N_COLS, N_ROWS, /*nBands=*/1,
        dst);
    assert(st == 0);

    for (int i = 0; i < N_PIXELS; ++i) {
        assert(dst_valid[i] == valid[i]);
    }
    for (int i = 0; i < N_PIXELS; ++i) {
        if (valid[i]) {
            assert(dst[i] == (double)src[i]);
        }
    }

    printf("lerc mask/decodeToDouble OK: %u bytes, mask preserved over %d pixels\n",
           written, N_PIXELS);
    return 0;
}
