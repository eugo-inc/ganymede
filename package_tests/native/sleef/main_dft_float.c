/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sleef: dft float; calls SleefDFT_float_init1d(), SleefDFT_float_execute(), SleefDFT_dispose().
 */

#include <sleef.h>
#include <sleefdft.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const int n = 8;
    /* Interleaved complex floats: real, imag, real, imag, ... */
    float *in  = (float *)calloc((size_t)(2 * n), sizeof(float));
    float *mid = (float *)calloc((size_t)(2 * n), sizeof(float));
    float *out = (float *)calloc((size_t)(2 * n), sizeof(float));
    assert(in && mid && out);

    /* Single spike at index 0; DFT of a delta is a flat spectrum. */
    in[0] = 1.0f;

    struct SleefDFT *fwd = SleefDFT_float_init1d(
        (uint32_t)n, in, mid, SLEEF_MODE_FORWARD | SLEEF_MODE_ESTIMATE);
    assert(fwd != NULL);
    SleefDFT_float_execute(fwd, NULL, NULL);

    struct SleefDFT *bwd = SleefDFT_float_init1d(
        (uint32_t)n, mid, out, SLEEF_MODE_BACKWARD | SLEEF_MODE_ESTIMATE);
    assert(bwd != NULL);
    SleefDFT_float_execute(bwd, NULL, NULL);

    assert(fabsf(out[0] / (float)n - 1.0f) < 1e-5f);
    for (int k = 1; k < n; ++k) {
        assert(fabsf(out[2 * k]) < 1e-5f);
        assert(fabsf(out[2 * k + 1]) < 1e-5f);
    }

    SleefDFT_dispose(fwd);
    SleefDFT_dispose(bwd);
    free(in);
    free(mid);
    free(out);

    printf("sleef dft float OK\n");
    return 0;
}
