/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for qhull: furthest site; calls qh_new_qhull(), qh_setvoronoi_all(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 4 corners of a unit square. */
    coordT points[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    };
    int dim = 2;
    int numpoints = 4;
    boolT ismalloc = False;
    /* `v` = Voronoi, `Qu` = furthest-site (UPPER envelope of paraboloid),
     * `Qbb` = scale paraboloid (Qhull's default for Delaunay/Voronoi).
     * `QJ` = joggle: the 4 cocircular square corners are rejected (QH6239)
     * for an exact lift; QJ is Qhull's recommended remedy (note `Qz` is NOT
     * usable here — it is incompatible with `Qu` furthest-site). The joggle
     * perturbs coordinates by ~1e-11, so the centroid lands at (0.5, 0.5)
     * only to ~1e-9 — the tolerance below is loosened to match. */
    char flags[] = "qhull v Qu Qbb QJ";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    qh_setvoronoi_all(qh);

    int furthest_centers_found = 0;
    facetT *facet;
    FORALLfacets {
        if (!facet->upperdelaunay) continue;
        if (facet->center == NULL) continue;
        double dx = facet->center[0] - 0.5;
        double dy = facet->center[1] - 0.5;
        assert(fabs(dx) < 1e-6);
        assert(fabs(dy) < 1e-6);
        furthest_centers_found++;
    }
    assert(furthest_centers_found >= 1);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull furthest-site voronoi test OK: %d upper-envelope centers\n",
           furthest_centers_found);
    return 0;
}
