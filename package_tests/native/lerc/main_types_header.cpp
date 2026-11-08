/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lerc: types header; calls lerc_computeCompressedSize(), lerc_encode(), lerc_getBlobInfo(), lerc_decode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <Lerc_types.h>
#include <Lerc_c_api.h>
#include <cassert>
#include <cstdio>
#include <cstring>

#define N_COLS 4
#define N_ROWS 4
#define N_PIXELS (N_COLS * N_ROWS)

int main(void) {
    /* Sanity: the C++ enum value for double must match the integer the C API uses. */
    static_assert(static_cast<int>(LercNS::DataType::dt_double) == 7,
                  "LercNS::DataType::dt_double must equal 7 (the C ABI value).");

    double src[N_PIXELS];
    for (int i = 0; i < N_PIXELS; ++i) {
        src[i] = static_cast<double>(i) * 1.25 + 0.5;
    }

    /* dataType 7 = double. */
    unsigned int needed = 0;
    lerc_status st = lerc_computeCompressedSize(
        src, /*dataType=*/7, /*nDepth=*/1, N_COLS, N_ROWS,
        /*nBands=*/1, /*nMasks=*/0, NULL, /*maxZErr=*/0.0, &needed);
    assert(st == 0);
    assert(needed > 0);

    unsigned char blob[16384];
    assert(needed <= sizeof(blob));
    unsigned int written = 0;
    st = lerc_encode(
        src, 7, 1, N_COLS, N_ROWS, 1, 0, NULL, 0.0,
        blob, sizeof(blob), &written);
    assert(st == 0);
    assert(written > 0);

    /* Blob header must agree the payload was double-typed. */
    unsigned int info[11] = {0};
    double ranges[3] = {0};
    st = lerc_getBlobInfo(blob, written, info, ranges, 11, 3);
    assert(st == 0);
    assert(info[1] == 7);     /* dataType == double */
    assert(info[3] == N_COLS);
    assert(info[4] == N_ROWS);

    double dst[N_PIXELS];
    memset(dst, 0, sizeof(dst));
    st = lerc_decode(blob, written, 0, NULL, 1, N_COLS, N_ROWS, 1, 7, dst);
    assert(st == 0);
    for (int i = 0; i < N_PIXELS; ++i) {
        assert(dst[i] == src[i]);
    }

    printf("lerc Lerc_types.h header + double codec OK: %u bytes\n", written);
    return 0;
}
