/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for qhull: halfspace; calls qh_new_qhull(), qh_freeqhull(), qh_memfreeshort().
 */

#include <libqhull_r/libqhull_r.h>
#include <libqhull_r/qhull_ra.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int dim = 2;
    int numhalfspaces = 4;
    /* (dim+1) coords per halfspace: [a0, a1, b] with Ax + b <= 0. */
    coordT points[12] = {
         1.0,  0.0, -1.0,   /* x <= 1 */
        -1.0,  0.0, -1.0,   /* -x <= 1, i.e. x >= -1 */
         0.0,  1.0, -1.0,   /* y <= 1 */
         0.0, -1.0, -1.0,   /* -y <= 1, i.e. y >= -1 */
    };
    boolT ismalloc = False;
    /* `H0,0` = halfspace intersection about the interior point (0,0).
     * qh_new_qhull receives dim+1 as its dimension argument — matches
     * user_eg_r.c's "qh_new_qhull(qh, dim+1, numpoints, ...)" pattern. */
    char flags[] = "qhull H0,0";

    qhT qh_qh;
    qhT *qh = &qh_qh;
    QHULL_LIB_CHECK
    qh_zero(qh, stderr);

    int exitcode = qh_new_qhull(qh, dim + 1, numhalfspaces, points, ismalloc,
                                flags, NULL, stderr);
    assert(exitcode == 0);

    assert(qh->num_vertices == 4);
    assert(qh->num_facets == 4);

    qh_freeqhull(qh, !qh_ALL);
    int curlong = 0, totlong = 0;
    qh_memfreeshort(qh, &curlong, &totlong);
    assert(curlong == 0 && totlong == 0);

    printf("qhull halfspace test OK: %d halfspaces -> %d dual facets\n",
           numhalfspaces, 4);
    return 0;
}
