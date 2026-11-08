/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sleef: simd; calls Sleef_sind2_u10(), Sleef_cosd2_u10(), vgetq_lane_f64(), Sleef_sinf4_u10().
 */

#include <sleef.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
#if defined(__aarch64__)
    /* AdvSIMD (NEON) — always available on AArch64. */
    float64x2_t v = {0.0, M_PI};
    float64x2_t s = Sleef_sind2_u10(v);
    float64x2_t c = Sleef_cosd2_u10(v);
    double s0 = vgetq_lane_f64(s, 0);
    double s1 = vgetq_lane_f64(s, 1);
    double c0 = vgetq_lane_f64(c, 0);
    double c1 = vgetq_lane_f64(c, 1);

    float32x4_t vf = {0.0f, (float)M_PI, (float)(M_PI / 2.0), 1.0f};
    float32x4_t sf = Sleef_sinf4_u10(vf);
    float sf0 = vgetq_lane_f32(sf, 0);
    float sf2 = vgetq_lane_f32(sf, 2);
#elif defined(__x86_64__) || defined(_M_X64)
    /* SSE2 — guaranteed on x86_64. */
    __m128d v = _mm_set_pd(M_PI, 0.0);
    __m128d s = Sleef_sind2_u10(v);
    __m128d c = Sleef_cosd2_u10(v);
    double sbuf[2], cbuf[2];
    _mm_storeu_pd(sbuf, s);
    _mm_storeu_pd(cbuf, c);
    double s0 = sbuf[0], s1 = sbuf[1];
    double c0 = cbuf[0], c1 = cbuf[1];

    __m128 vf = _mm_set_ps(1.0f, (float)(M_PI / 2.0), (float)M_PI, 0.0f);
    __m128 sf = Sleef_sinf4_u10(vf);
    float sfbuf[4];
    _mm_storeu_ps(sfbuf, sf);
    float sf0 = sfbuf[0], sf2 = sfbuf[2];
#else
#error "Unsupported architecture for SLEEF SIMD smoke test"
#endif

    /* sin(0) == 0,  sin(pi) ~= 0,  cos(0) == 1,  cos(pi) == -1 */
    assert(fabs(s0) < 1e-9);
    assert(fabs(s1) < 1e-9);
    assert(fabs(c0 - 1.0) < 1e-9);
    assert(fabs(c1 + 1.0) < 1e-9);

    /* single-precision: sinf(0)==0, sinf(pi/2)==1 */
    assert(fabsf(sf0) < 1e-5f);
    assert(fabsf(sf2 - 1.0f) < 1e-5f);

    printf("sleef simd OK\n");
    return 0;
}
