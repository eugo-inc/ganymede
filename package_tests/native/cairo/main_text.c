/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cairo: text; calls cairo_image_surface_create(), cairo_surface_status(), cairo_create(), cairo_status().
 */

#include <cairo/cairo.h>
#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    const int W = 64, H = 32;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);

    cairo_t *cr = cairo_create(surface);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    /* Clear background to opaque black so any glyph rendering shows up. */
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    /* --- Toy font selection. Family "sans" should always resolve via fontconfig. --- */
    cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 16.0);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    /* --- Text extents must be non-zero for a non-empty string. --- */
    cairo_text_extents_t ext;
    const char *text = "Demo";
    cairo_text_extents(cr, text, &ext);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);
    assert(ext.width > 0.0);
    assert(ext.height > 0.0);
    assert(ext.x_advance > 0.0);

    /* --- Font extents are also non-trivial. --- */
    cairo_font_extents_t fe;
    cairo_font_extents(cr, &fe);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);
    assert(fe.ascent > 0.0);
    assert(fe.height > 0.0);

    /* --- Render the text. Must touch at least one pixel away from pure black. --- */
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 2.0, 20.0);
    cairo_show_text(cr, text);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    cairo_surface_flush(surface);
    unsigned char *data = cairo_image_surface_get_data(surface);
    assert(data != NULL);
    int stride = cairo_image_surface_get_stride(surface);

    int nonblack_pixels = 0;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            uint32_t px = *((uint32_t *)(data + y * stride + x * 4));
            uint8_t r = (px >> 16) & 0xFF;
            uint8_t g = (px >> 8) & 0xFF;
            uint8_t b = px & 0xFF;
            if (r > 0 || g > 0 || b > 0) {
                nonblack_pixels++;
            }
        }
    }
    assert(nonblack_pixels > 0);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    cairo_debug_reset_static_data();
    FcFini();

    printf("cairo text test OK: extents=%.2fx%.2f, %d non-black pixels\n",
           ext.width, ext.height, nonblack_pixels);
    return 0;
}
