/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for geos: prepared; calls initGEOS(), GEOSWKTReader_create(), GEOSWKTReader_read(), GEOSPrepare().
 */

#include <geos_c.h>
#include <assert.h>
#include <stdio.h>

static void notice_handler(const char *fmt, ...) { (void)fmt; }

int main(void) {
    initGEOS(notice_handler, notice_handler);

    GEOSWKTReader *r = GEOSWKTReader_create();
    assert(r != NULL);

    GEOSGeometry *poly = GEOSWKTReader_read(r,
        "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
    assert(poly != NULL);

    const GEOSPreparedGeometry *prepared = GEOSPrepare(poly);
    assert(prepared != NULL);

    /* Interior point: (5, 5) — clearly inside the 10x10 square. */
    GEOSGeometry *inside_point = GEOSWKTReader_read(r, "POINT(5 5)");
    assert(inside_point != NULL);
    char contains_inside = GEOSPreparedContains(prepared, inside_point);
    assert(contains_inside == 1);

    /* Exterior point: (20, 20) — clearly outside. */
    GEOSGeometry *outside_point = GEOSWKTReader_read(r, "POINT(20 20)");
    assert(outside_point != NULL);
    char contains_outside = GEOSPreparedContains(prepared, outside_point);
    assert(contains_outside == 0);

    /* Overlapping polygon: shifted so it overlaps the prepared polygon. */
    GEOSGeometry *overlapper = GEOSWKTReader_read(r,
        "POLYGON((5 5, 15 5, 15 15, 5 15, 5 5))");
    assert(overlapper != NULL);
    char intersects = GEOSPreparedIntersects(prepared, overlapper);
    assert(intersects == 1);

    GEOSGeom_destroy(overlapper);
    GEOSGeom_destroy(outside_point);
    GEOSGeom_destroy(inside_point);
    /* PreparedGeometry must be destroyed BEFORE the underlying geometry. */
    GEOSPreparedGeom_destroy(prepared);
    GEOSGeom_destroy(poly);
    GEOSWKTReader_destroy(r);

    finishGEOS();

    printf("geos prepared-geometry OK\n");
    return 0;
}
