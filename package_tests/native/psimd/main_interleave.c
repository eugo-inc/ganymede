/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for psimd: interleave; calls psimd_load_f32(), psimd_interleave_lo_f32(), psimd_interleave_hi_f32(), psimd_store_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    float a_buf[4] __attribute__((aligned(16))) = {1.0f, 2.0f, 3.0f, 4.0f};
    float b_buf[4] __attribute__((aligned(16))) = {5.0f, 6.0f, 7.0f, 8.0f};
    const psimd_f32 a = psimd_load_f32(a_buf);
    const psimd_f32 b = psimd_load_f32(b_buf);

    /* interleave_lo: take low halves of a={1,2,3,4} and b={5,6,7,8} ->
     * {1,5,2,6}. */
    const psimd_f32 lo = psimd_interleave_lo_f32(a, b);
    float out[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(out, lo);
    assert(out[0] == 1.0f);
    assert(out[1] == 5.0f);
    assert(out[2] == 2.0f);
    assert(out[3] == 6.0f);

    /* interleave_hi: take high halves -> {3,7,4,8}. */
    const psimd_f32 hi = psimd_interleave_hi_f32(a, b);
    psimd_store_f32(out, hi);
    assert(out[0] == 3.0f);
    assert(out[1] == 7.0f);
    assert(out[2] == 4.0f);
    assert(out[3] == 8.0f);

    /* concat_lo: low half of a + low half of b -> {1,2,5,6}. */
    const psimd_f32 clo = psimd_concat_lo_f32(a, b);
    psimd_store_f32(out, clo);
    assert(out[0] == 1.0f);
    assert(out[1] == 2.0f);
    assert(out[2] == 5.0f);
    assert(out[3] == 6.0f);

    /* concat_hi: high half of a + high half of b -> {3,4,7,8}. */
    const psimd_f32 chi = psimd_concat_hi_f32(a, b);
    psimd_store_f32(out, chi);
    assert(out[0] == 3.0f);
    assert(out[1] == 4.0f);
    assert(out[2] == 7.0f);
    assert(out[3] == 8.0f);

    /* concat_even: even-indexed lanes of a then b -> {1,3,5,7}. */
    const psimd_f32 ceven = psimd_concat_even_f32(a, b);
    psimd_store_f32(out, ceven);
    assert(out[0] == 1.0f);
    assert(out[1] == 3.0f);
    assert(out[2] == 5.0f);
    assert(out[3] == 7.0f);

    /* concat_odd: odd-indexed lanes of a then b -> {2,4,6,8}. */
    const psimd_f32 codd = psimd_concat_odd_f32(a, b);
    psimd_store_f32(out, codd);
    assert(out[0] == 2.0f);
    assert(out[1] == 4.0f);
    assert(out[2] == 6.0f);
    assert(out[3] == 8.0f);

    printf("psimd interleave test OK\n");
    return 0;
}
