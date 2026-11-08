/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for qhull: 3d hull; calls qh_new_qhull(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 9 points in 3D: the 8 corners of a unit cube + the centroid (interior). */
    coordT points[27] = {
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.5, 0.5, 0.5,
    };
    int dim = 3;
    int numpoints = 9;
    boolT ismalloc = False;
    char flags[] = "qhull s";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    /* Unit cube: 8 vertices, 6 faces (the interior centroid is excluded). */
    assert(qh->num_vertices == 8);
    assert(qh->num_facets == 6);

    /* Walk every facet via the FORALL iterator (distinct API surface from
     * the simple num_facets field check). Each facet must have a unit-length
     * normal (within Qhull's tolerance) — otherwise the hull is degenerate. */
    int facets_visited = 0;
    facetT *facet;
    FORALLfacets {
        assert(facet->normal != NULL);
        double n2 = facet->normal[0] * facet->normal[0]
                  + facet->normal[1] * facet->normal[1]
                  + facet->normal[2] * facet->normal[2];
        /* Normal must be unit length (Qhull normalizes). */
        assert(n2 > 0.99 && n2 < 1.01);
        facets_visited++;
    }
    assert(facets_visited == 6);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull 3d-hull test OK: %d facets visited\n", facets_visited);
    return 0;
}
