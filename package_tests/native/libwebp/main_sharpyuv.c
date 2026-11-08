/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libwebp: sharpyuv; calls SharpYuvGetVersion(), SharpYuvGetConversionMatrix(), SharpYuvConvert().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <webp/sharpyuv/sharpyuv.h>
#include <webp/sharpyuv/sharpyuv_csp.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const uint8_t rgb[4 * 2 * 3] = {
        0xff, 0x00, 0x00,  0x00, 0xff, 0x00,  0x00, 0x00, 0xff,  0xff, 0xff, 0xff,
        0x10, 0x20, 0x30,  0x40, 0x50, 0x60,  0x70, 0x80, 0x90,  0xa0, 0xb0, 0xc0,
    };

    uint8_t y_plane[4 * 2] = {0};
    uint8_t u_plane[2 * 1] = {0};
    uint8_t v_plane[2 * 1] = {0};

    int version = SharpYuvGetVersion();
    assert(version > 0);

    /* kSharpYuvMatrixWebp is a SharpYuvMatrixType enum, so it selects a
     * predefined matrix via SharpYuvGetConversionMatrix (which returns a
     * pointer). SharpYuvComputeConversionMatrix is a different API that takes a
     * SharpYuvColorSpace* — passing the enum (== 0) there would be a NULL
     * deref. */
    const SharpYuvConversionMatrix* matrix =
        SharpYuvGetConversionMatrix(kSharpYuvMatrixWebp);
    assert(matrix != NULL);

    int rc = SharpYuvConvert(
        /* r_ptr */ rgb + 0, /* g_ptr */ rgb + 1, /* b_ptr */ rgb + 2,
        /* rgb_step */ 3, /* rgb_stride */ 4 * 3, /* rgb_bit_depth */ 8,
        y_plane, /* y_stride */ 4,
        u_plane, /* u_stride */ 2,
        v_plane, /* v_stride */ 2,
        /* yuv_bit_depth */ 8,
        /* width */ 4, /* height */ 2,
        matrix);
    assert(rc == 1);

    /* At least one Y sample must be non-zero (input is not all black). */
    int any_y = 0;
    for (size_t i = 0; i < sizeof(y_plane); i++) {
        if (y_plane[i] != 0) { any_y = 1; break; }
    }
    assert(any_y);

    printf("libsharpyuv test OK: version=0x%06x\n", version);
    return 0;
}
