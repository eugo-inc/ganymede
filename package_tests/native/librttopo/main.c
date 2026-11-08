/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for librttopo: core API; calls rtgeom_init(), rtpoint_make2d(), rtgeom_free(), rtgeom_finish().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <librttopo_geom.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    RTCTX *ctx = rtgeom_init(malloc, realloc, free);
    assert(ctx != NULL);

    /* Construct a 2D point at (1.0, 2.0) in an arbitrary SRID — exercises the
     * geometry constructor path + the RTCTX-bound allocator wiring. */
    RTPOINT *p = rtpoint_make2d(ctx, 4326, 1.0, 2.0);
    assert(p != NULL);

    /* Cast to RTGEOM to use the polymorphic destructor (the documented pattern). */
    rtgeom_free(ctx, (RTGEOM *)p);

    rtgeom_finish(ctx);

    printf("librttopo test OK\n");
    return 0;
}
