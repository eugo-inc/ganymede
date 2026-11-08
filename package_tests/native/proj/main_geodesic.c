/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for proj: geodesic; calls proj_context_create(), proj_create(), proj_coord(), proj_geod().
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

    PJ *P = proj_create(ctx, "+proj=longlat +datum=WGS84 +no_defs");
    assert(P != NULL);

    /* proj_geod(P, a, b): solves inverse geodesic on the ellipsoid of P.
     * Inputs MUST be (lon, lat) in RADIANS (regardless of CRS axis order).
     * Returns distance in meters in .xy.x (per the PROJ 9.x API). */
    const double D2R = M_PI / 180.0;
    PJ_COORD paris = proj_coord(2.3522 * D2R, 48.8566 * D2R, 0.0, 0.0);
    PJ_COORD nyc = proj_coord(-74.0060 * D2R, 40.7128 * D2R, 0.0, 0.0);
    PJ_COORD g = proj_geod(P, paris, nyc);
    double distance_m = g.xy.x;

    assert(isfinite(distance_m));
    assert(distance_m > 0.0);
    /* Paris ↔ NYC WGS84 ellipsoidal geodesic is ~5,853,000 m (PROJ/GeographicLib
     * returns 5,852,935 m). Tolerance ±5000 m absorbs any minor coordinate-rounding
     * error in the inputs above. */
    assert(fabs(distance_m - 5853000.0) < 5000.0);

    proj_destroy(P);
    proj_context_destroy(ctx);

    printf("proj geodesic OK: Paris<->NYC = %.0f m\n", distance_m);
    return 0;
}
