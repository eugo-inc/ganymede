/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lcms2: tone curve; calls cmsBuildGamma(), cmsEvalToneCurveFloat(), cmsEvalToneCurve16(), cmsIsToneCurveMonotonic().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <lcms2.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    cmsToneCurve *curve = cmsBuildGamma(NULL, 2.2);
    assert(curve != NULL);

    /* Endpoints: f(0) == 0, f(1) == 1 for any positive gamma. */
    cmsFloat32Number y0 = cmsEvalToneCurveFloat(curve, 0.0f);
    cmsFloat32Number y1 = cmsEvalToneCurveFloat(curve, 1.0f);
    assert(fabsf(y0 - 0.0f) < 1e-4f);
    assert(fabsf(y1 - 1.0f) < 1e-3f);

    /* Midpoint: f(0.5) == 0.5 ** 2.2 ~= 0.2176. */
    cmsFloat32Number ymid = cmsEvalToneCurveFloat(curve, 0.5f);
    cmsFloat32Number expected = powf(0.5f, 2.2f);
    assert(fabsf(ymid - expected) < 1e-2f);

    /* 16-bit evaluator should agree with the float evaluator at midpoint. */
    cmsUInt16Number y16 = cmsEvalToneCurve16(curve, 0x8000);
    cmsFloat32Number y16_norm = (cmsFloat32Number)y16 / 65535.0f;
    assert(fabsf(y16_norm - expected) < 1e-2f);

    /* Curve must report itself as monotonic. */
    assert(cmsIsToneCurveMonotonic(curve));

    cmsToneCurve *reversed = cmsReverseToneCurve(curve);
    assert(reversed != NULL);
    cmsFloat32Number round_trip = cmsEvalToneCurveFloat(reversed, ymid);
    assert(fabsf(round_trip - 0.5f) < 1e-2f);

    cmsFreeToneCurve(reversed);
    cmsFreeToneCurve(curve);

    printf("lcms2 tone-curve test OK: gamma(0.5)=%.4f\n", ymid);
    return 0;
}
