/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ta_lib: core API; calls TA_Initialize(), TA_GetVersionString(), TA_Shutdown().
 */

#include <ta-lib/ta_libc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef TA_RetCode (*ta_unary_fn)(int, int, const double[], int *, int *, double[]);

static int near(double a, double b) {
    double d = a - b;
    return (d < 0 ? -d : d) <= 1e-12;
}

/* Runs a two-element vector transform and fails loudly, naming the function and every value. */
static int check_unary(const char *name, ta_unary_fn fn, const double in[2], const double expected[2]) {
    double out[2] = {0.0, 0.0};
    int beg = -1;
    int n = -1;
    TA_RetCode rc = fn(0, 1, in, &beg, &n, out);
    if (rc != TA_SUCCESS || beg != 0 || n != 2 || !near(out[0], expected[0]) || !near(out[1], expected[1])) {
        fprintf(stderr, "ta_lib test FAILED: %s rc=%d outBegIdx=%d outNbElement=%d out=[%.17g, %.17g] expected=[%.17g, %.17g]\n",
                name, (int)rc, beg, n, out[0], out[1], expected[0], expected[1]);
        return 1;
    }
    return 0;
}

int main(void) {
    assert(TA_Initialize() == TA_SUCCESS);

    const char *v = TA_GetVersionString();
    assert(v != NULL);
    assert(v[0] >= '0' && v[0] <= '9');

    /* Simple Moving Average over [1,2,3,4,5] with period 3 yields [2,3,4]. */
    const double input[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double output[5] = {0};
    int outBegIdx = -1;
    int outNbElement = -1;
    TA_RetCode rc = TA_SMA(0, 4, input, 3, &outBegIdx, &outNbElement, output);
    assert(rc == TA_SUCCESS);
    assert(outBegIdx == 2);
    assert(outNbElement == 3);
    assert(output[0] == 2.0);
    assert(output[1] == 3.0);
    assert(output[2] == 4.0);

    /* libm-backed transforms: exp(0)=1, exp(1)=e; log(1)=0, log(e)=1; cos(0)=1, cos(pi)=-1. */
    const double exp_in[2] = {0.0, 1.0};
    const double exp_expected[2] = {1.0, 2.718281828459045};
    const double ln_in[2] = {1.0, 2.718281828459045};
    const double ln_expected[2] = {0.0, 1.0};
    const double cos_in[2] = {0.0, 3.141592653589793};
    const double cos_expected[2] = {1.0, -1.0};
    int failures = 0;
    failures += check_unary("TA_EXP", TA_EXP, exp_in, exp_expected);
    failures += check_unary("TA_LN", TA_LN, ln_in, ln_expected);
    failures += check_unary("TA_COS", TA_COS, cos_in, cos_expected);

    TA_Shutdown();

    if (failures != 0) {
        return 1;
    }
    printf("ta_lib test OK: %s\n", v);
    return 0;
}
