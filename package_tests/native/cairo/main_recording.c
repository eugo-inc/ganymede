/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cairo: recording; calls cairo_recording_surface_create(), cairo_surface_status(), cairo_surface_get_type(), cairo_create().
 */

#include <cairo/cairo.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    cairo_rectangle_t extents_hint = {0.0, 0.0, 16.0, 16.0};
    cairo_surface_t *rec = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &extents_hint);
    assert(rec != NULL);
    assert(cairo_surface_status(rec) == CAIRO_STATUS_SUCCESS);
    assert(cairo_surface_get_type(rec) == CAIRO_SURFACE_TYPE_RECORDING);

    cairo_t *rec_cr = cairo_create(rec);
    assert(cairo_status(rec_cr) == CAIRO_STATUS_SUCCESS);

    cairo_set_source_rgba(rec_cr, 0.0, 1.0, 0.0, 1.0);
    cairo_rectangle(rec_cr, 4.0, 4.0, 8.0, 8.0);
    cairo_fill(rec_cr);
    assert(cairo_status(rec_cr) == CAIRO_STATUS_SUCCESS);
    cairo_destroy(rec_cr);

    /* --- Ink extents should report the painted region (a non-empty bbox). --- */
    double ink_x, ink_y, ink_w, ink_h;
    cairo_recording_surface_ink_extents(rec, &ink_x, &ink_y, &ink_w, &ink_h);
    assert(ink_w > 0.0);
    assert(ink_h > 0.0);

    cairo_surface_t *img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
    assert(cairo_surface_status(img) == CAIRO_STATUS_SUCCESS);

    cairo_t *img_cr = cairo_create(img);
    cairo_set_operator(img_cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(img_cr);
    cairo_set_operator(img_cr, CAIRO_OPERATOR_OVER);

    cairo_set_source_surface(img_cr, rec, 0.0, 0.0);
    cairo_paint(img_cr);
    assert(cairo_status(img_cr) == CAIRO_STATUS_SUCCESS);
    cairo_destroy(img_cr);

    /* --- Inspect the replayed pixels. (8,8) is inside the green square. --- */
    cairo_surface_flush(img);
    unsigned char *data = cairo_image_surface_get_data(img);
    assert(data != NULL);
    int stride = cairo_image_surface_get_stride(img);

    uint32_t inside = *((uint32_t *)(data + 8 * stride + 8 * 4));
    uint8_t a = (inside >> 24) & 0xFF;
    uint8_t r = (inside >> 16) & 0xFF;
    uint8_t g = (inside >> 8) & 0xFF;
    uint8_t b = inside & 0xFF;
    assert(a == 0xFF);
    assert(g > 200);  /* Green channel dominates. */
    assert(r < 60);
    assert(b < 60);

    /* (0,0) is outside the green square — should still be transparent. */
    uint32_t outside = *((uint32_t *)(data + 0));
    uint8_t out_a = (outside >> 24) & 0xFF;
    assert(out_a == 0x00);

    cairo_surface_destroy(img);
    cairo_surface_destroy(rec);

    printf("cairo recording test OK: ink_extents=%.1fx%.1f at (%.1f,%.1f)\n",
           ink_w, ink_h, ink_x, ink_y);
    return 0;
}
