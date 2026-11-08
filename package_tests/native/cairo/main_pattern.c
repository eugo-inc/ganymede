/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cairo: pattern; calls cairo_image_surface_create(), cairo_surface_status(), cairo_create(), cairo_status().
 */

#include <cairo/cairo.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    const int W = 32, H = 1;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);

    cairo_t *cr = cairo_create(surface);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    /* Horizontal gradient: opaque red (left) -> opaque blue (right). */
    cairo_pattern_t *pat = cairo_pattern_create_linear(0.0, 0.0, (double)W, 0.0);
    assert(cairo_pattern_status(pat) == CAIRO_STATUS_SUCCESS);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 0.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.0, 0.0, 1.0, 1.0);

    int n_stops = 0;
    cairo_status_t cs = cairo_pattern_get_color_stop_count(pat, &n_stops);
    assert(cs == CAIRO_STATUS_SUCCESS);
    assert(n_stops == 2);

    cairo_set_source(cr, pat);
    cairo_paint(cr);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    /* Read back pixels. ARGB32 is native-endian uint32: 0xAARRGGBB on little-endian. */
    cairo_surface_flush(surface);
    unsigned char *data = cairo_image_surface_get_data(surface);
    assert(data != NULL);
    int stride = cairo_image_surface_get_stride(surface);
    assert(stride >= W * 4);

    uint32_t left = *((uint32_t *)(data + 0));
    uint32_t right = *((uint32_t *)(data + (W - 1) * 4));

    uint8_t left_a = (left >> 24) & 0xFF;
    uint8_t left_r = (left >> 16) & 0xFF;
    uint8_t left_g = (left >> 8) & 0xFF;
    uint8_t left_b = left & 0xFF;
    uint8_t right_a = (right >> 24) & 0xFF;
    uint8_t right_r = (right >> 16) & 0xFF;
    uint8_t right_g = (right >> 8) & 0xFF;
    uint8_t right_b = right & 0xFF;

    /* Left edge: dominantly red. */
    assert(left_a == 0xFF);
    assert(left_r > 200);
    assert(left_g < 60);
    assert(left_b < 60);

    /* Right edge: dominantly blue. */
    assert(right_a == 0xFF);
    assert(right_r < 60);
    assert(right_g < 60);
    assert(right_b > 200);

    cairo_pattern_destroy(pat);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    printf("cairo pattern test OK\n");
    return 0;
}
