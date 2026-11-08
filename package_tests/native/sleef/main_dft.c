/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sleef: dft; calls SleefDFT_double_init1d(), SleefDFT_double_execute(), SleefDFT_dispose().
 */

#include <sleef.h>
#include <sleefdft.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const int n = 8;
    /* interleaved complex doubles: real, imag, real, imag, ... */
    double *in = (double *)calloc((size_t)(2 * n), sizeof(double));
    double *mid = (double *)calloc((size_t)(2 * n), sizeof(double));
    double *out = (double *)calloc((size_t)(2 * n), sizeof(double));
    assert(in && mid && out);

    /* Single-spike at index 0; DFT of a delta is a flat spectrum. */
    in[0] = 1.0;

    struct SleefDFT *fwd = SleefDFT_double_init1d(
        (uint32_t)n, in, mid, SLEEF_MODE_FORWARD | SLEEF_MODE_ESTIMATE);
    assert(fwd != NULL);
    SleefDFT_double_execute(fwd, NULL, NULL);

    struct SleefDFT *bwd = SleefDFT_double_init1d(
        (uint32_t)n, mid, out, SLEEF_MODE_BACKWARD | SLEEF_MODE_ESTIMATE);
    assert(bwd != NULL);
    SleefDFT_double_execute(bwd, NULL, NULL);

    assert(fabs(out[0] / (double)n - 1.0) < 1e-9);
    for (int k = 1; k < n; ++k) {
        assert(fabs(out[2 * k]) < 1e-9);
        assert(fabs(out[2 * k + 1]) < 1e-9);
    }

    SleefDFT_dispose(fwd);
    SleefDFT_dispose(bwd);
    free(in);
    free(mid);
    free(out);

    printf("sleef dft OK\n");
    return 0;
}
