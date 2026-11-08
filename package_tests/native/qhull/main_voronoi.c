/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for qhull: voronoi; calls qh_new_qhull(), qh_setvoronoi_all(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 4 corners of a unit square — Voronoi diagram has exactly 1 finite
     * vertex at the centroid (0.5, 0.5). */
    coordT points[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    };
    int dim = 2;
    int numpoints = 4;
    boolT ismalloc = False;
    /* `v` = Voronoi, `Qbb` = scale paraboloid (qvoronoi's defaults).
     * `QJ` = joggle: the 4 cocircular square corners are rejected (QH6239)
     * for an exact Delaunay/Voronoi lift; QJ is Qhull's recommended remedy.
     * The joggle perturbs coordinates by ~1e-11, so the Voronoi vertex lands
     * at (0.5, 0.5) only to ~1e-9 — the tolerance below is loosened to match. */
    char flags[] = "qhull v Qbb QJ";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    /* qh_setvoronoi_all populates facet->center with the Voronoi vertex
     * coordinates (the circumcenter of each lower-envelope Delaunay facet).
     * This is the API surface qvoronoi's output stage relies on. */
    qh_setvoronoi_all(qh);

    int finite_centers_found = 0;
    facetT *facet;
    FORALLfacets {
        if (facet->upperdelaunay) continue;
        if (facet->center == NULL) continue;
        double dx = facet->center[0] - 0.5;
        double dy = facet->center[1] - 0.5;
        assert(fabs(dx) < 1e-6);
        assert(fabs(dy) < 1e-6);
        finite_centers_found++;
    }
    assert(finite_centers_found >= 1);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull voronoi test OK: %d centers at (0.5, 0.5)\n",
           finite_centers_found);
    return 0;
}
