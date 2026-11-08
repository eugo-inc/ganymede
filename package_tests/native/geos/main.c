/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for geos: core API; calls initGEOS(), GEOSWKTReader_create(), GEOSWKTReader_read(), GEOSIntersection().
 */

#include <geos_c.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void notice_handler(const char *fmt, ...) { (void)fmt; }

int main(void) {
    initGEOS(notice_handler, notice_handler);

    GEOSWKTReader *reader = GEOSWKTReader_create();
    assert(reader != NULL);

    /* Two unit squares offset so they overlap on a 0.5 x 0.5 region (area = 0.25). */
    GEOSGeometry *a = GEOSWKTReader_read(reader, "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))");
    GEOSGeometry *b = GEOSWKTReader_read(reader, "POLYGON((0.5 0.5, 1.5 0.5, 1.5 1.5, 0.5 1.5, 0.5 0.5))");
    assert(a != NULL);
    assert(b != NULL);

    GEOSGeometry *inter = GEOSIntersection(a, b);
    assert(inter != NULL);

    double area = 0.0;
    assert(GEOSArea(inter, &area) == 1);
    assert(fabs(area - 0.25) < 1e-9);

    GEOSGeom_destroy(inter);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(a);
    GEOSWKTReader_destroy(reader);

    const char *v = GEOSversion();
    assert(v != NULL);
    assert(strlen(v) > 0);

    finishGEOS();

    printf("geos test OK: %s\n", v);
    return 0;
}
