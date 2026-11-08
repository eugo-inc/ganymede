/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pixman: composite; calls pixman_image_create_bits(), pixman_image_composite32(), pixman_image_unref().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    /* Destination: 2x2 opaque red (premultiplied: A=R=0xff). */
    uint32_t dst_pixels[4] = {
        0xffff0000u, 0xffff0000u,
        0xffff0000u, 0xffff0000u
    };
    pixman_image_t *dst = pixman_image_create_bits(
        PIXMAN_a8r8g8b8, 2, 2, dst_pixels, 2 * sizeof(uint32_t));
    assert(dst != NULL);

    /* Source: 2x2 50%-alpha blue, premultiplied (A=0x80, B=0x80). */
    uint32_t src_pixels[4] = {
        0x80000080u, 0x80000080u,
        0x80000080u, 0x80000080u
    };
    pixman_image_t *src = pixman_image_create_bits(
        PIXMAN_a8r8g8b8, 2, 2, src_pixels, 2 * sizeof(uint32_t));
    assert(src != NULL);

    /* SRC-OVER composite: dst = src + dst * (1 - src.a). */
    pixman_image_composite32(PIXMAN_OP_OVER, src, NULL, dst,
                             0, 0, 0, 0, 0, 0, 2, 2);

    /* Expected per channel (premultiplied):
     *   A = 0x80 + 0xff * (1 - 0x80/0xff) = 0xff
     *   R = 0x00 + 0xff * (1 - 0x80/0xff) ≈ 0x7f
     *   G = 0
     *   B = 0x80 + 0x00 = 0x80
     * Pixman uses a `div_255`-like rounding so allow +/- 1 on R.
     */
    for (int i = 0; i < 4; i++) {
        uint32_t px = dst_pixels[i];
        uint8_t a = (px >> 24) & 0xff;
        uint8_t r = (px >> 16) & 0xff;
        uint8_t g = (px >>  8) & 0xff;
        uint8_t b = (px      ) & 0xff;
        assert(a == 0xff);
        assert(r >= 0x7e && r <= 0x80);
        assert(g == 0x00);
        assert(b == 0x80);
    }

    pixman_image_unref(src);
    pixman_image_unref(dst);

    printf("pixman composite test OK\n");
    return 0;
}
