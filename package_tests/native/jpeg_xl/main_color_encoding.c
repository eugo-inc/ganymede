/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jpeg_xl: color encoding; calls JxlColorEncodingSetToSRGB(), JxlColorEncodingSetToLinearSRGB().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jxl/color_encoding.h>
#include <jxl/encode.h>
#include <jxl/types.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    JxlColorEncoding srgb;
    JxlColorEncodingSetToSRGB(&srgb, JXL_FALSE /* is_gray */);

    assert(srgb.color_space == JXL_COLOR_SPACE_RGB);
    assert(srgb.white_point == JXL_WHITE_POINT_D65);
    assert(srgb.primaries == JXL_PRIMARIES_SRGB);
    assert(srgb.transfer_function == JXL_TRANSFER_FUNCTION_SRGB);

    JxlColorEncoding linear;
    JxlColorEncodingSetToLinearSRGB(&linear, JXL_FALSE);
    assert(linear.color_space == JXL_COLOR_SPACE_RGB);
    assert(linear.white_point == JXL_WHITE_POINT_D65);
    assert(linear.transfer_function == JXL_TRANSFER_FUNCTION_LINEAR);

    /* Grayscale sRGB variant should report a gray color space. */
    JxlColorEncoding gray;
    JxlColorEncodingSetToSRGB(&gray, JXL_TRUE /* is_gray */);
    assert(gray.color_space == JXL_COLOR_SPACE_GRAY);

    printf("jpeg_xl color encoding test OK\n");
    return 0;
}
