/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for proj: pipeline; calls proj_context_create(), proj_create(), proj_coord(), proj_trans().
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

    /* Pipeline that converts (lon_deg, lat_deg) → (lon_rad, lat_rad). The unitconvert
     * step is purely arithmetic; no database lookup needed. */
    const char *pipeline =
        "+proj=pipeline "
        "+step +proj=unitconvert +xy_in=deg +xy_out=rad";
    PJ *P = proj_create(ctx, pipeline);
    assert(P != NULL);

    /* 90° → π/2 rad on x, 45° → π/4 rad on y. */
    PJ_COORD in = proj_coord(90.0, 45.0, 0.0, 0.0);
    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    assert(isfinite(out.xy.x));
    assert(isfinite(out.xy.y));
    assert(fabs(out.xy.x - (M_PI / 2.0)) < 1.0e-12);
    assert(fabs(out.xy.y - (M_PI / 4.0)) < 1.0e-12);

    /* Inverse step must undo the conversion exactly (rational rad→deg factor). */
    PJ_COORD back = proj_trans(P, PJ_INV, out);
    assert(fabs(back.xy.x - 90.0) < 1.0e-9);
    assert(fabs(back.xy.y - 45.0) < 1.0e-9);

    proj_destroy(P);
    proj_context_destroy(ctx);

    printf("proj pipeline OK\n");
    return 0;
}
