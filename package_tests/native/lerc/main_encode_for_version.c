/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lerc: encode for version; calls lerc_computeCompressedSizeForVersion(), lerc_encodeForVersion(), lerc_getBlobInfo(), lerc_decode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_c_api.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_COLS 5
#define N_ROWS 5
#define N_PIXELS (N_COLS * N_ROWS)

static void check_roundtrip_at_version(int codec_version, const short *src) {
    unsigned int needed = 0;
    lerc_status st = lerc_computeCompressedSizeForVersion(
        src, codec_version, /*dataType=*/2 /* short */,
        /*nDepth=*/1, N_COLS, N_ROWS, /*nBands=*/1,
        /*nMasks=*/0, NULL, /*maxZErr=*/0.0, &needed);
    assert(st == 0);
    assert(needed > 0);

    unsigned char blob[32768];
    assert(needed <= sizeof(blob));
    unsigned int written = 0;
    st = lerc_encodeForVersion(
        src, codec_version, 2, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0,
        blob, sizeof(blob), &written);
    assert(st == 0);
    assert(written > 0);

    unsigned int info[11] = {0};
    double ranges[3] = {0};
    st = lerc_getBlobInfo(blob, written, info, ranges, 11, 3);
    assert(st == 0);
    assert((int)info[0] == codec_version);
    assert(info[1] == 2);          /* dataType == short */

    short dst[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    st = lerc_decode(blob, written, 0, NULL, 1, N_COLS, N_ROWS, 1, 2, dst);
    assert(st == 0);
    for (int i = 0; i < N_PIXELS; ++i) {
        assert(dst[i] == src[i]);
    }
}

int main(void) {
    short src[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        src[i] = (short)((i * 31) - 100);
    }

    check_roundtrip_at_version(4, src);
    check_roundtrip_at_version(5, src);

    printf("lerc encodeForVersion OK: codec versions 4 and 5 both round-tripped\n");
    return 0;
}
