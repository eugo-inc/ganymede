/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fp16: rounding; calls ldexpf(), fp16_ieee_from_fp32_value().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* 0.25 ULP above 1.0 -> rounds down to 0x3C00. */
    const float quarter_ulp_above_one = 1.0f + ldexpf(1.0f, -12);
    const uint16_t r_quarter = fp16_ieee_from_fp32_value(quarter_ulp_above_one);
    assert(r_quarter == UINT16_C(0x3C00));

    /* Exact half-ULP tie above 1.0 -> RNE picks the even neighbor.
     * 0x3C00 (LSB=0) is even; 0x3C01 (LSB=1) is odd. So the tie rounds DOWN. */
    const float half_ulp_above_one = 1.0f + ldexpf(1.0f, -11);
    const uint16_t r_tie = fp16_ieee_from_fp32_value(half_ulp_above_one);
    assert(r_tie == UINT16_C(0x3C00));

    /* 0.75 ULP above 1.0 -> rounds up to 0x3C01. */
    const float three_quarter_ulp_above_one = 1.0f + 3.0f * ldexpf(1.0f, -12);
    const uint16_t r_three_quarter = fp16_ieee_from_fp32_value(three_quarter_ulp_above_one);
    assert(r_three_quarter == UINT16_C(0x3C01));

    const float tie_at_3C01_3C02 = 1.0f + ldexpf(1.0f, -10) + ldexpf(1.0f, -11);
    const uint16_t r_tie_up = fp16_ieee_from_fp32_value(tie_at_3C01_3C02);
    assert(r_tie_up == UINT16_C(0x3C02));

    printf("fp16 rounding test OK\n");
    return 0;
}
