/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for librttopo: geojson; calls rtgeom_init(), rtpoint_make2d(), rtpoint_as_rtgeom(), rtgeom_get_type().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <librttopo_geom.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    RTCTX *ctx = rtgeom_init(malloc, realloc, free);
    assert(ctx != NULL);

    /* Construct a 2D point at (7.0, 8.0), SRID=4326. */
    RTPOINT *p = rtpoint_make2d(ctx, 4326, 7.0, 8.0);
    assert(p != NULL);
    RTGEOM *g = rtpoint_as_rtgeom(ctx, p);
    assert(g != NULL);

    /* Introspect type + SRID — verifies the accessor wiring. */
    assert(rtgeom_get_type(ctx, g) == RTPOINTTYPE);
    assert(rtgeom_get_srid(ctx, g) == 4326);

    /* Serialize to GeoJSON. precision=2 keeps the digits stable.
     * srs=NULL → no top-level "crs" field; has_bbox=0 → no bbox field. */
    char *geojson = rtgeom_to_geojson(ctx, g, NULL, 2, 0);
    assert(geojson != NULL);

    assert(strstr(geojson, "Point") != NULL);
    assert(strstr(geojson, "7") != NULL);
    assert(strstr(geojson, "8") != NULL);

    /* Clean up. */
    rtfree(ctx, geojson);
    rtgeom_free(ctx, g);
    rtgeom_finish(ctx);

    printf("librttopo GeoJSON OK\n");
    return 0;
}
