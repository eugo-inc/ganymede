/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fp16: bitcasts; calls fp32_from_bits(), fp32_to_bits(), fp64_from_bits(), fp64_to_bits().
 */

#include <fp16/bitcasts.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* fp32: bit pattern of 1.0f is 0x3F800000. */
    const float one_f = fp32_from_bits(UINT32_C(0x3F800000));
    assert(one_f == 1.0f);

    const uint32_t one_f_bits = fp32_to_bits(1.0f);
    assert(one_f_bits == UINT32_C(0x3F800000));

    /* fp64: bit pattern of 1.0 is 0x3FF0000000000000. */
    const double one_d = fp64_from_bits(UINT64_C(0x3FF0000000000000));
    assert(one_d == 1.0);

    const uint64_t one_d_bits = fp64_to_bits(1.0);
    assert(one_d_bits == UINT64_C(0x3FF0000000000000));

    const float v = -2.71828f;
    const uint32_t v_bits = fp32_to_bits(v);
    const float v_back = fp32_from_bits(v_bits);
    assert(v_back == v);

    const double d = -3.14159265358979;
    const uint64_t d_bits = fp64_to_bits(d);
    const double d_back = fp64_from_bits(d_bits);
    assert(d_back == d);

    printf("fp16 bitcasts test OK\n");
    return 0;
}
