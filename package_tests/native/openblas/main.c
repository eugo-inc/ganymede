/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openblas: core API; calls cblas_ddot(), cblas_dgemm(), openblas_get_config().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <cblas.h>
#include <openblas_config.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    /* Dot product: [1,2,3,4] . [4,3,2,1] = 4 + 6 + 6 + 4 = 20. */
    double x[4] = {1.0, 2.0, 3.0, 4.0};
    double y[4] = {4.0, 3.0, 2.0, 1.0};
    double dot = cblas_ddot(4, x, 1, y, 1);
    assert(fabs(dot - 20.0) < 1e-12);

    /* C = A * B, 2x2 row-major. A = [[1,2],[3,4]], B = [[5,6],[7,8]] => C = [[19,22],[43,50]]. */
    double A[4] = {1.0, 2.0, 3.0, 4.0};
    double B[4] = {5.0, 6.0, 7.0, 8.0};
    double C[4] = {0.0, 0.0, 0.0, 0.0};
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                2, 2, 2, 1.0, A, 2, B, 2, 0.0, C, 2);
    assert(fabs(C[0] - 19.0) < 1e-12);
    assert(fabs(C[1] - 22.0) < 1e-12);
    assert(fabs(C[2] - 43.0) < 1e-12);
    assert(fabs(C[3] - 50.0) < 1e-12);

    const char *cfg = openblas_get_config();
    assert(cfg != NULL);

    printf("openblas test OK: %s\n", cfg);
    return 0;
}
