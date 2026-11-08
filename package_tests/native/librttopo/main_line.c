/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for librttopo: line; calls rtgeom_init(), ptarray_construct_empty(), ptarray_append_point(), rtline_construct().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <librttopo_geom.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(void) {
    RTCTX *ctx = rtgeom_init(malloc, realloc, free);
    assert(ctx != NULL);

    RTPOINTARRAY *pa = ptarray_construct_empty(ctx, 0, 0, 2);
    assert(pa != NULL);

    RTPOINT4D p0 = {0.0, 0.0, 0.0, 0.0};
    RTPOINT4D p1 = {1.0, 1.0, 0.0, 0.0};
    int r0 = ptarray_append_point(ctx, pa, &p0, 1 /* allow_duplicates */);
    int r1 = ptarray_append_point(ctx, pa, &p1, 1);
    assert(r0 == RT_SUCCESS);
    assert(r1 == RT_SUCCESS);

    /* Construct the line (no bbox, SRID=4326). */
    RTLINE *ln = rtline_construct(ctx, 4326, NULL, pa);
    assert(ln != NULL);
    RTGEOM *g = rtline_as_rtgeom(ctx, ln);
    assert(g != NULL);
    assert(rtgeom_get_type(ctx, g) == RTLINETYPE);

    /* Cartesian length of (0,0)->(1,1) is sqrt(2). Tolerance is loose enough
     * that any double-precision implementation will pass; tight enough that
     * a "return 0" stub would fail. */
    double len = rtgeom_length(ctx, g);
    assert(fabs(len - sqrt(2.0)) < 1e-9);

    /* Clean up. rtgeom_free walks the line + its owned point array. */
    rtgeom_free(ctx, g);
    rtgeom_finish(ctx);

    printf("librttopo line length OK (got %.12f, expected sqrt(2))\n", len);
    return 0;
}
