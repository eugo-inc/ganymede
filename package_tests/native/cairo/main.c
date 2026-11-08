/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cairo: core API; calls cairo_image_surface_create(), cairo_surface_status(), cairo_image_surface_get_width(), cairo_image_surface_get_height().
 */

#include <cairo/cairo.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
    assert(surface != NULL);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);
    assert(cairo_image_surface_get_width(surface) == 16);
    assert(cairo_image_surface_get_height(surface) == 16);

    cairo_t *cr = cairo_create(surface);
    assert(cr != NULL);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    /* Fill with opaque red, then stroke a path — exercises the raster + path engines. */
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, 16.0, 16.0);
    cairo_stroke(cr);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    const char *v = cairo_version_string();
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("cairo test OK: %s\n", v);
    return 0;
}
