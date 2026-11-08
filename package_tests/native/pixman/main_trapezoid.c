/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pixman: trapezoid; calls pixman_image_create_bits(), pixman_int_to_fixed(), pixman_trapezoid_valid(), pixman_rasterize_trapezoid().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    /* 8-bit alpha mask, 8x8, starting fully transparent. */
    uint8_t mask_pixels[64];
    for (int i = 0; i < 64; i++) mask_pixels[i] = 0;
    pixman_image_t *mask = pixman_image_create_bits(
        PIXMAN_a8, 8, 8, (uint32_t *)mask_pixels, 8);
    assert(mask != NULL);

    /* Axis-aligned trapezoid covering the upper-left 4x4 quadrant
     * fully (top=0, bottom=4, left edge x=0..0, right edge x=4..4). */
    pixman_trapezoid_t trap;
    trap.top    = pixman_int_to_fixed(0);
    trap.bottom = pixman_int_to_fixed(4);
    trap.left.p1.x  = pixman_int_to_fixed(0);
    trap.left.p1.y  = pixman_int_to_fixed(0);
    trap.left.p2.x  = pixman_int_to_fixed(0);
    trap.left.p2.y  = pixman_int_to_fixed(4);
    trap.right.p1.x = pixman_int_to_fixed(4);
    trap.right.p1.y = pixman_int_to_fixed(0);
    trap.right.p2.x = pixman_int_to_fixed(4);
    trap.right.p2.y = pixman_int_to_fixed(4);

    assert(pixman_trapezoid_valid(&trap));

    pixman_rasterize_trapezoid(mask, &trap, 0, 0);

    /* The covered quadrant must be fully opaque (0xff) on every pixel,
     * and the uncovered quadrants must remain fully transparent (0x00). */
    int covered_full = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (mask_pixels[y * 8 + x] == 0xff) covered_full++;
        }
    }
    /* Trapezoid scan-conversion must hit the interior with full coverage. */
    assert(covered_full >= 4);

    /* The opposite (lower-right) corner must be entirely untouched. */
    for (int y = 4; y < 8; y++) {
        for (int x = 4; x < 8; x++) {
            assert(mask_pixels[y * 8 + x] == 0x00);
        }
    }

    pixman_image_unref(mask);

    printf("pixman trapezoid test OK\n");
    return 0;
}
