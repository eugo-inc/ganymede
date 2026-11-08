/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for proj: ellipsoid; calls proj_context_create(), proj_create_from_database(), proj_get_ellipsoid(), proj_ellipsoid_get_parameters().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <proj.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    PJ_CONTEXT *ctx = proj_context_create();
    assert(ctx != NULL);

    /* Load EPSG:4326 (WGS84 geographic) from the projection database. */
    PJ *crs = proj_create_from_database(ctx, "EPSG", "4326",
                                        PJ_CATEGORY_CRS, 0, NULL);
    assert(crs != NULL);

    /* Walk to the ellipsoid object. */
    PJ *ellipsoid = proj_get_ellipsoid(ctx, crs);
    assert(ellipsoid != NULL);

    double a = 0.0, b = 0.0, inv_flat = 0.0;
    int semi_minor_computed = -1;
    int ok = proj_ellipsoid_get_parameters(ctx, ellipsoid,
                                           &a, &b, &semi_minor_computed, &inv_flat);
    assert(ok == 1);
    assert(fabs(a - 6378137.0) < 1.0e-3);            /* WGS84 semi-major */
    assert(fabs(inv_flat - 298.257223563) < 1.0e-6); /* WGS84 inverse flattening */
    assert(b > 6356000.0 && b < 6357000.0);
    assert(semi_minor_computed == 0 || semi_minor_computed == 1); /* must be a valid bool */

    proj_destroy(ellipsoid);
    proj_destroy(crs);
    proj_context_destroy(ctx);

    printf("proj ellipsoid metadata OK: a=%.3f 1/f=%.9f\n", a, inv_flat);
    return 0;
}
