/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cairo: path; calls cairo_matrix_init_identity(), cairo_matrix_translate(), cairo_matrix_scale(), cairo_matrix_transform_point().
 */

#include <cairo/cairo.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int approx_eq(double a, double b) {
    double d = a - b;
    if (d < 0) d = -d;
    return d < 1e-9;
}

int main(void) {
    /* --- Affine matrix composition --- */
    cairo_matrix_t m;
    cairo_matrix_init_identity(&m);
    cairo_matrix_translate(&m, 10.0, 20.0);
    cairo_matrix_scale(&m, 2.0, 3.0);

    double x = 1.0, y = 1.0;
    cairo_matrix_transform_point(&m, &x, &y);
    /* Order: translate THEN scale in the local frame:
     *   (1,1) -> scale -> (2,3) -> translate -> (12,23) */
    assert(approx_eq(x, 12.0));
    assert(approx_eq(y, 23.0));

    cairo_status_t inv = cairo_matrix_invert(&m);
    assert(inv == CAIRO_STATUS_SUCCESS);
    cairo_matrix_transform_point(&m, &x, &y);
    assert(approx_eq(x, 1.0));
    assert(approx_eq(y, 1.0));

    /* --- Path inspection --- */
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
    cairo_t *cr = cairo_create(surface);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    cairo_move_to(cr, 1.0, 2.0);
    cairo_line_to(cr, 3.0, 4.0);
    cairo_close_path(cr);

    cairo_path_t *path = cairo_copy_path(cr);
    assert(path != NULL);
    assert(path->status == CAIRO_STATUS_SUCCESS);
    assert(path->num_data >= 4);  /* MOVE_TO(2) + LINE_TO(2) at minimum */

    cairo_path_data_t *d = &path->data[0];
    assert(d->header.type == CAIRO_PATH_MOVE_TO);
    assert(d->header.length == 2);
    /* d[1] is the (x,y) for MOVE_TO */
    assert(approx_eq(path->data[1].point.x, 1.0));
    assert(approx_eq(path->data[1].point.y, 2.0));

    /* Next chunk should be LINE_TO. */
    cairo_path_data_t *d2 = &path->data[d->header.length];
    assert(d2->header.type == CAIRO_PATH_LINE_TO);
    assert(approx_eq(path->data[d->header.length + 1].point.x, 3.0));
    assert(approx_eq(path->data[d->header.length + 1].point.y, 4.0));

    cairo_path_destroy(path);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    printf("cairo path/matrix test OK\n");
    return 0;
}
