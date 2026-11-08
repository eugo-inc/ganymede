/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fp16: core API; calls fp16_ieee_to_fp32_bits(), fp16_ieee_to_fp32_value(), fp16_ieee_from_fp32_value().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* 0x3C00 is the IEEE-754 half-precision encoding of 1.0. */
    const uint16_t one_half = UINT16_C(0x3C00);

    /* Half -> float: bit pattern of 1.0f is 0x3F800000. */
    const uint32_t f32_bits = fp16_ieee_to_fp32_bits(one_half);
    assert(f32_bits == UINT32_C(0x3F800000));

    /* Half -> float value: must equal 1.0f exactly. */
    const float f32_value = fp16_ieee_to_fp32_value(one_half);
    assert(f32_value == 1.0f);

    const uint16_t round_trip = fp16_ieee_from_fp32_value(1.0f);
    assert(round_trip == one_half);

    printf("fp16 test OK\n");
    return 0;
}
