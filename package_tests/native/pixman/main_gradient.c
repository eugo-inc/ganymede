/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pixman: gradient; calls pixman_int_to_fixed(), pixman_image_create_linear_gradient(), pixman_image_create_bits(), pixman_image_composite32().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    pixman_point_fixed_t p1 = { pixman_int_to_fixed(0), pixman_int_to_fixed(0) };
    pixman_point_fixed_t p2 = { pixman_int_to_fixed(4), pixman_int_to_fixed(0) };
    pixman_gradient_stop_t stops[2];
    stops[0].x = 0;
    stops[0].color.alpha = 0xffff;
    stops[0].color.red   = 0x0000;
    stops[0].color.green = 0x0000;
    stops[0].color.blue  = 0x0000;
    stops[1].x = pixman_int_to_fixed(1);
    stops[1].color.alpha = 0xffff;
    stops[1].color.red   = 0xffff;
    stops[1].color.green = 0xffff;
    stops[1].color.blue  = 0xffff;

    pixman_image_t *grad = pixman_image_create_linear_gradient(&p1, &p2, stops, 2);
    assert(grad != NULL);

    /* 4x1 ARGB32 destination. */
    uint32_t dst_pixels[4] = { 0, 0, 0, 0 };
    pixman_image_t *dst = pixman_image_create_bits(
        PIXMAN_a8r8g8b8, 4, 1, dst_pixels, 4 * sizeof(uint32_t));
    assert(dst != NULL);

    /* Paint the gradient onto the destination. */
    pixman_image_composite32(PIXMAN_OP_SRC, grad, NULL, dst,
                             0, 0, 0, 0, 0, 0, 4, 1);

    uint8_t prev_r = 0;
    int saw_change = 0;
    for (int i = 0; i < 4; i++) {
        uint32_t px = dst_pixels[i];
        uint8_t a = (px >> 24) & 0xff;
        uint8_t r = (px >> 16) & 0xff;
        uint8_t g = (px >>  8) & 0xff;
        uint8_t b = (px      ) & 0xff;
        assert(a == 0xff);
        assert(r == g);
        assert(g == b);
        assert(r >= prev_r);
        if (r != prev_r) saw_change = 1;
        prev_r = r;
    }
    /* A real gradient must produce at least one channel transition. */
    assert(saw_change);

    pixman_image_unref(grad);
    pixman_image_unref(dst);

    printf("pixman gradient test OK\n");
    return 0;
}
