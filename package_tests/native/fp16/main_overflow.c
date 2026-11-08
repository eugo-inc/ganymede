/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fp16: overflow; calls fp16_ieee_from_fp32_value(), fp16_ieee_to_fp32_value().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* Large finite positive fp32 -> +inf in half. */
    const uint16_t big_pos = fp16_ieee_from_fp32_value(70000.0f);
    assert(big_pos == UINT16_C(0x7C00));

    /* Even larger value -> still +inf. */
    const uint16_t huge_pos = fp16_ieee_from_fp32_value(1e30f);
    assert(huge_pos == UINT16_C(0x7C00));

    /* Large finite negative fp32 -> -inf in half. */
    const uint16_t big_neg = fp16_ieee_from_fp32_value(-1e30f);
    assert(big_neg == UINT16_C(0xFC00));

    /* Tiny positive fp32 (well below smallest subnormal 2^-24) -> +0 in half. */
    const uint16_t tiny_pos = fp16_ieee_from_fp32_value(1e-10f);
    assert(tiny_pos == UINT16_C(0x0000));

    /* Tiny negative fp32 -> -0 in half. Compare via bits (since +0 == -0). */
    const uint16_t tiny_neg = fp16_ieee_from_fp32_value(-1e-10f);
    assert(tiny_neg == UINT16_C(0x8000));

    const float max_half = fp16_ieee_to_fp32_value(UINT16_C(0x7BFF));
    assert(max_half == 65504.0f);
    const uint16_t max_back = fp16_ieee_from_fp32_value(65504.0f);
    assert(max_back == UINT16_C(0x7BFF));

    /* One ULP above max-finite half (65504 + ULP/2 = 65520) is the IEEE-754
     * rounding boundary -> rounds UP to +inf. Use 65536.0 which is safely above. */
    const uint16_t above_max = fp16_ieee_from_fp32_value(65536.0f);
    assert(above_max == UINT16_C(0x7C00));

    printf("fp16 overflow test OK\n");
    return 0;
}
