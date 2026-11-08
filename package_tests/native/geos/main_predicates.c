/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for geos: predicates; calls initGEOS(), GEOSWKTReader_create(), GEOSWKTReader_read(), GEOSBuffer().
 */

#include <geos_c.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void notice_handler(const char *fmt, ...) { (void)fmt; }

int main(void) {
    initGEOS(notice_handler, notice_handler);

    GEOSWKTReader *r = GEOSWKTReader_create();
    assert(r != NULL);

    /* --- Buffer: point at origin, radius 1.0, ~pi area --- */
    GEOSGeometry *point = GEOSWKTReader_read(r, "POINT(0 0)");
    assert(point != NULL);

    /* 64 quadrant segments is enough to get within ~0.1% of pi. */
    GEOSGeometry *buf = GEOSBuffer(point, 1.0, 64);
    assert(buf != NULL);

    double area = 0.0;
    assert(GEOSArea(buf, &area) == 1);
    /* Buffer of radius 1 should approximate pi (3.14159...). */
    assert(fabs(area - M_PI) < 0.01);

    /* --- Contains: a 10x10 polygon should contain a 1x1 polygon inside it --- */
    GEOSGeometry *outer = GEOSWKTReader_read(r,
        "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
    GEOSGeometry *inner = GEOSWKTReader_read(r,
        "POLYGON((1 1, 2 1, 2 2, 1 2, 1 1))");
    assert(outer != NULL);
    assert(inner != NULL);

    char contains = GEOSContains(outer, inner);
    assert(contains == 1);

    /* --- Disjoint + Intersects: two squares 100 units apart --- */
    GEOSGeometry *far_a = GEOSWKTReader_read(r,
        "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))");
    GEOSGeometry *far_b = GEOSWKTReader_read(r,
        "POLYGON((100 100, 101 100, 101 101, 100 101, 100 100))");
    assert(far_a != NULL);
    assert(far_b != NULL);

    char disjoint = GEOSDisjoint(far_a, far_b);
    char intersects = GEOSIntersects(far_a, far_b);
    assert(disjoint == 1);
    assert(intersects == 0);

    GEOSGeom_destroy(far_b);
    GEOSGeom_destroy(far_a);
    GEOSGeom_destroy(inner);
    GEOSGeom_destroy(outer);
    GEOSGeom_destroy(buf);
    GEOSGeom_destroy(point);
    GEOSWKTReader_destroy(r);

    finishGEOS();

    printf("geos buffer/predicates OK (buffer area=%.6f, pi=%.6f)\n", area, M_PI);
    return 0;
}
