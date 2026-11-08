/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for qhull: core API; calls qh_new_qhull(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 5 points in 2D: the 4 corners of a unit square + the centroid (interior). */
    coordT points[10] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.5, 0.5,
    };
    int dim = 2;
    int numpoints = 5;
    boolT ismalloc = False;
    char flags[] = "qhull s";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    /* The hull of a unit square (with an interior point) must have 4 vertices and 4 facets. */
    assert(qh->num_vertices == 4);
    assert(qh->num_facets == 4);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull test OK: %s\n", qh_version);
    return 0;
}
