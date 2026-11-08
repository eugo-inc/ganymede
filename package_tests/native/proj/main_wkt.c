/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for proj: wkt; calls proj_context_create(), proj_create_from_database(), proj_as_wkt(), proj_destroy().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <proj.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    PJ_CONTEXT *ctx = proj_context_create();
    assert(ctx != NULL);

    /* Load EPSG:4326 (WGS84 geographic) from the projection database. */
    PJ *crs = proj_create_from_database(ctx, "EPSG", "4326",
                                        PJ_CATEGORY_CRS, 0, NULL);
    assert(crs != NULL);

    /* Emit WKT2:2019. proj_as_wkt returns a context-owned C string (NUL-terminated). */
    const char *wkt = proj_as_wkt(ctx, crs, PJ_WKT2_2019, NULL);
    assert(wkt != NULL);
    assert(strlen(wkt) > 0);

    assert(strstr(wkt, "WGS 84") != NULL);
    /* WKT2:2019 uses "GEOGCRS"; accept either spelling defensively. */
    assert(strstr(wkt, "GEOGCRS") != NULL || strstr(wkt, "GeodeticCRS") != NULL);

    /* The ellipsoid block must reference the WGS 84 ellipsoid by name. */
    assert(strstr(wkt, "ELLIPSOID") != NULL || strstr(wkt, "Ellipsoid") != NULL);

    assert(strstr(wkt, "4326") != NULL);

    const size_t wkt_len = strlen(wkt);

    proj_destroy(crs);
    proj_context_destroy(ctx);

    printf("proj WKT export OK (%zu bytes)\n", wkt_len);
    return 0;
}
