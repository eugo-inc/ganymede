/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sleef: scalar math; calls Sleef_log_u10(), Sleef_exp_u10(), Sleef_pow_u10(), Sleef_cosf_u10().
 */

#include <sleef.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    /* log(exp(x)) == x within u10 ulp at x=1.0 */
    double x = 1.0;
    double back = Sleef_log_u10(Sleef_exp_u10(x));
    assert(fabs(back - x) < 1e-9);

    /* pow(2, 10) == 1024 */
    double p = Sleef_pow_u10(2.0, 10.0);
    assert(fabs(p - 1024.0) < 1e-6);

    /* single-precision cos(0) == 1 */
    float c = Sleef_cosf_u10(0.0f);
    assert(fabsf(c - 1.0f) < 1e-6f);

    printf("sleef scalar math OK\n");
    return 0;
}
