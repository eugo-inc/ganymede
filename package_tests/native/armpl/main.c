/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for armpl: core API; calls _ZGVnN2v_cos(), vgetq_lane_f64().
 */

#include <arm_neon.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* libmvec aarch64 Advanced-SIMD ABI: vector cos for two doubles per the
 * Vector Function ABI specification for AArch64 (`n` = NEON, `N2` = 2 lanes,
 * `v` = value-passed vector argument). */
extern float64x2_t _ZGVnN2v_cos(float64x2_t);

int main(void) {
    const double in0 = 0.0;
    const double in1 = 1.0;

    float64x2_t v = {in0, in1};
    float64x2_t r = _ZGVnN2v_cos(v);

    const double r0 = vgetq_lane_f64(r, 0);
    const double r1 = vgetq_lane_f64(r, 1);

    /* Compare against libm scalar cos — libmvec accuracy is ULP-bounded; 1e-9
     * is well within the documented tolerance for vector math kernels. */
    assert(fabs(r0 - cos(in0)) < 1e-9);
    assert(fabs(r1 - cos(in1)) < 1e-9);

    printf("armpl test OK: libamath _ZGVnN2v_cos({%g,%g}) = {%g,%g}\n", in0, in1, r0, r1);
    return 0;
}
