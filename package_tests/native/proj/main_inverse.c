/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for proj: inverse; calls proj_context_create(), proj_create_crs_to_crs(), proj_coord(), proj_trans().
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

    PJ *P = proj_create_crs_to_crs(ctx, "EPSG:4326", "EPSG:3857", NULL);
    assert(P != NULL);

    /* Pick a non-trivial point: Greenwich Observatory, ~(51.4778 N, 0.0015 W).
     * EPSG:4326 axis order is (lat, lon) for proj_create_crs_to_crs by default in PROJ 9.x. */
    PJ_COORD origin_geo = proj_coord(51.4778, -0.0015, 0.0, 0.0);

    /* Forward: geo → projected. */
    PJ_COORD projected = proj_trans(P, PJ_FWD, origin_geo);
    /* Web Mercator x at this longitude should be small (~ -167 m), y should be the projected
     * Mercator northing in millions of meters at ~51.5°N (positive, well above the equator). */
    assert(isfinite(projected.xy.x));
    assert(isfinite(projected.xy.y));
    assert(projected.xy.y > 1.0e6); /* north of equator, well into the northern hemisphere */
    assert(fabs(projected.xy.x) < 1.0e3); /* longitude was near the meridian */

    /* Inverse: projected → geo. Must recover the original lat/lon to high precision. */
    PJ_COORD recovered = proj_trans(P, PJ_INV, projected);
    assert(fabs(recovered.lp.lam - origin_geo.lp.lam) < 1.0e-9);
    assert(fabs(recovered.lp.phi - origin_geo.lp.phi) < 1.0e-9);

    proj_destroy(P);
    proj_context_destroy(ctx);

    printf("proj inverse+round-trip OK\n");
    return 0;
}
