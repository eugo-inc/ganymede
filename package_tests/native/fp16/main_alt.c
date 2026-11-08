/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fp16: alt; calls fp16_alt_to_fp32_bits(), fp16_alt_to_fp32_value(), fp16_alt_from_fp32_value().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* In ARM-alt half, 0x3C00 still encodes 1.0 (same as IEEE half for normals
     * within IEEE's representable range). The difference manifests only at
     * exp=31. */
    const uint16_t alt_one = UINT16_C(0x3C00);

    const uint32_t f32_bits = fp16_alt_to_fp32_bits(alt_one);
    assert(f32_bits == UINT32_C(0x3F800000));

    const float f32_value = fp16_alt_to_fp32_value(alt_one);
    assert(f32_value == 1.0f);

    const uint16_t round_trip = fp16_alt_from_fp32_value(1.0f);
    assert(round_trip == alt_one);

    /* Sign-bit preservation: -1.0f -> 0xBC00 -> -1.0f. */
    const uint16_t alt_neg_one = UINT16_C(0xBC00);
    const float neg_value = fp16_alt_to_fp32_value(alt_neg_one);
    assert(neg_value == -1.0f);

    const uint16_t neg_round_trip = fp16_alt_from_fp32_value(-1.0f);
    assert(neg_round_trip == alt_neg_one);

    printf("fp16 alt-format test OK\n");
    return 0;
}
