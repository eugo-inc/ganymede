/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for qhull: delaunay; calls qh_new_qhull(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 4 corners of a unit square in 2D — Delaunay yields exactly 2 triangles. */
    coordT points[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    };
    int dim = 2;
    int numpoints = 4;
    boolT ismalloc = False;
    char flags[] = "qhull d Qbb QJ";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    /* Count only the lower-envelope facets — those ARE the Delaunay
     * triangles. Upper facets are an artifact of the paraboloid lift. */
    int delaunay_triangles = 0;
    facetT *facet;
    FORALLfacets {
        if (!facet->upperdelaunay) {
            delaunay_triangles++;
        }
    }
    assert(delaunay_triangles == 2);

    /* All 4 input points must be vertices of the Delaunay triangulation. */
    assert(qh->num_vertices == 4);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull delaunay test OK: %d triangles\n", delaunay_triangles);
    return 0;
}
