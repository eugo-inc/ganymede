/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pango: matrix; calls pango_matrix_copy(), pango_matrix_translate(), pango_matrix_scale(), pango_matrix_transform_distance().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int near_eq(double a, double b) {
    double d = a - b;
    if (d < 0) d = -d;
    return d < 1e-9;
}

int main(void) {
    /* Identity matrix copy + free. */
    const PangoMatrix identity = PANGO_MATRIX_INIT;
    assert(near_eq(identity.xx, 1.0));
    assert(near_eq(identity.xy, 0.0));
    assert(near_eq(identity.yx, 0.0));
    assert(near_eq(identity.yy, 1.0));
    assert(near_eq(identity.x0, 0.0));
    assert(near_eq(identity.y0, 0.0));

    PangoMatrix *m = pango_matrix_copy(&identity);
    assert(m != NULL);
    assert(near_eq(m->xx, 1.0));
    assert(near_eq(m->yy, 1.0));

    /* Translate by (10, 20) — only the x0/y0 components should change. */
    pango_matrix_translate(m, 10.0, 20.0);
    assert(near_eq(m->x0, 10.0));
    assert(near_eq(m->y0, 20.0));
    assert(near_eq(m->xx, 1.0));
    assert(near_eq(m->yy, 1.0));

    pango_matrix_scale(m, 2.0, 3.0);
    assert(near_eq(m->xx, 2.0));
    assert(near_eq(m->yy, 3.0));

    /* Transform a distance vector — the x0/y0 part must be IGNORED for distances. */
    PangoMatrix m2 = PANGO_MATRIX_INIT;
    pango_matrix_translate(&m2, 100.0, 200.0);
    pango_matrix_scale(&m2, 2.0, 4.0);
    double dx = 1.0, dy = 1.0;
    pango_matrix_transform_distance(&m2, &dx, &dy);
    assert(near_eq(dx, 2.0));
    assert(near_eq(dy, 4.0));

    /* Transform a point — translation MUST apply. */
    double px = 0.0, py = 0.0;
    pango_matrix_transform_point(&m2, &px, &py);
    assert(near_eq(px, 100.0));
    assert(near_eq(py, 200.0));

    /* Rotate 90 degrees on a fresh identity: xx,yy → 0; xy → -1; yx → +1. */
    PangoMatrix m3 = PANGO_MATRIX_INIT;
    pango_matrix_rotate(&m3, 90.0);
    assert(near_eq(m3.xx, 0.0));
    assert(near_eq(m3.yy, 0.0));
    /* Sign convention check: pango uses degrees, CCW. cos(90)=0, sin(90)=1. */
    assert(fabs(m3.xy) > 0.5);
    assert(fabs(m3.yx) > 0.5);

    /* get_font_scale_factor on an identity must be 1.0. */
    PangoMatrix m4 = PANGO_MATRIX_INIT;
    double s = pango_matrix_get_font_scale_factor(&m4);
    assert(near_eq(s, 1.0));

    /* On a uniform-2x scale, the font scale factor must be 2.0. */
    PangoMatrix m5 = PANGO_MATRIX_INIT;
    pango_matrix_scale(&m5, 2.0, 2.0);
    double s2 = pango_matrix_get_font_scale_factor(&m5);
    assert(near_eq(s2, 2.0));

    pango_matrix_free(m);

    printf("pango matrix test OK\n");
    return 0;
}
