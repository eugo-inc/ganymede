/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for proj: core API; calls proj_context_create(), proj_create_crs_to_crs(), proj_coord(), proj_trans().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <proj.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    PJ_CONTEXT *ctx = proj_context_create();
    assert(ctx != NULL);

    /* WGS84 lat/lon → Web Mercator (meters). Canonical, exercised in every GIS stack. */
    PJ *P = proj_create_crs_to_crs(ctx, "EPSG:4326", "EPSG:3857", NULL);
    assert(P != NULL);

    PJ_COORD in = proj_coord(0.0, 0.0, 0.0, 0.0);
    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    assert(fabs(out.xy.x) < 1e-6);
    assert(fabs(out.xy.y) < 1e-6);

    proj_destroy(P);
    proj_context_destroy(ctx);

    const char *v = proj_info().version;
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("proj test OK: %s\n", v);
    return 0;
}
