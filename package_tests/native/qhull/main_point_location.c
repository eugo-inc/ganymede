/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for qhull: point location; calls qh_new_qhull(), qh_findbestfacet(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 4 corners of the unit square in 2D. */
    coordT points[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
    };
    int dim = 2;
    int numpoints = 4;
    boolT ismalloc = False;
    char flags[] = "qhull s";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim, numpoints, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);
    assert(qh->num_vertices == 4);
    assert(qh->num_facets == 4);

    coordT inside_point[2] = {0.5, 0.5};
    boolT isoutside_inside = True;  /* deliberately wrong initial value */
    realT bestdist_inside = 0.0;
    facetT *facet_inside = qh_findbestfacet(qh, inside_point, qh_ALL,
                                            &bestdist_inside,
                                            &isoutside_inside);
    assert(facet_inside != NULL);
    /* (0.5, 0.5) is strictly interior -> isoutside must flip to False. */
    assert(isoutside_inside == False);

    coordT outside_point[2] = {2.0, 2.0};
    boolT isoutside_outside = False;  /* deliberately wrong initial value */
    realT bestdist_outside = 0.0;
    facetT *facet_outside = qh_findbestfacet(qh, outside_point, qh_ALL,
                                             &bestdist_outside,
                                             &isoutside_outside);
    assert(facet_outside != NULL);
    /* (2.0, 2.0) is well outside -> isoutside must be True. */
    assert(isoutside_outside == True);
    /* And the signed distance to the best facet must be strictly positive
     * (the point is on the OUTSIDE side of every facet's hyperplane). */
    assert(bestdist_outside > 0.0);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull point-location test OK: inside=%d outside=%d (dist=%g)\n",
           (int)isoutside_inside, (int)isoutside_outside, bestdist_outside);
    return 0;
}
