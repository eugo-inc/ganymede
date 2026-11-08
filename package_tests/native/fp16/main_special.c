/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fp16: special; calls fp16_ieee_to_fp32_value(), fp16_ieee_to_fp32_bits(), fp16_ieee_from_fp32_value(), ldexpf().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* +0.0 -> 0x00000000 (positive zero). */
    const float pos_zero = fp16_ieee_to_fp32_value(UINT16_C(0x0000));
    assert(fp16_ieee_to_fp32_bits(UINT16_C(0x0000)) == UINT32_C(0x00000000));
    assert(pos_zero == 0.0f);

    /* -0.0 -> 0x80000000 (negative zero). Compare via bits since +0 == -0. */
    assert(fp16_ieee_to_fp32_bits(UINT16_C(0x8000)) == UINT32_C(0x80000000));

    /* +inf : half 0x7C00 (exp=31, mant=0) -> float 0x7F800000. */
    const float pos_inf = fp16_ieee_to_fp32_value(UINT16_C(0x7C00));
    assert(fp16_ieee_to_fp32_bits(UINT16_C(0x7C00)) == UINT32_C(0x7F800000));
    assert(isinf(pos_inf) && pos_inf > 0.0f);

    /* -inf : half 0xFC00 -> float 0xFF800000. */
    const float neg_inf = fp16_ieee_to_fp32_value(UINT16_C(0xFC00));
    assert(fp16_ieee_to_fp32_bits(UINT16_C(0xFC00)) == UINT32_C(0xFF800000));
    assert(isinf(neg_inf) && neg_inf < 0.0f);

    /* NaN : half 0x7E00 (exp=31, mant=0x200) must produce a float NaN. */
    const float nan_v = fp16_ieee_to_fp32_value(UINT16_C(0x7E00));
    assert(isnan(nan_v));

    const uint16_t inf_h = fp16_ieee_from_fp32_value((float) INFINITY);
    assert(inf_h == UINT16_C(0x7C00));

    /* Smallest positive subnormal half = 0x0001 = 2^-24. Exercises the denormal
     * renorm_shift path in the conversion code. 2^-24 in float bits = 0x33800000. */
    const uint32_t subnormal_bits = fp16_ieee_to_fp32_bits(UINT16_C(0x0001));
    assert(subnormal_bits == UINT32_C(0x33800000));
    const float subnormal_v = fp16_ieee_to_fp32_value(UINT16_C(0x0001));
    assert(subnormal_v > 0.0f);
    assert(subnormal_v == ldexpf(1.0f, -24));

    printf("fp16 special-values test OK\n");
    return 0;
}
