/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for psimd: minmax; calls psimd_load_f32(), psimd_max_f32(), psimd_min_f32(), psimd_store_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    float a_buf[4] __attribute__((aligned(16))) = { 1.0f, -2.0f,  3.0f, -4.0f };
    float b_buf[4] __attribute__((aligned(16))) = {-1.5f,  2.5f, -3.5f,  4.5f };

    const psimd_f32 a = psimd_load_f32(a_buf);
    const psimd_f32 b = psimd_load_f32(b_buf);

    /* Elementwise max: {1, 2.5, 3, 4.5}. */
    const psimd_f32 mx = psimd_max_f32(a, b);
    float out[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(out, mx);
    assert(out[0] == 1.0f);
    assert(out[1] == 2.5f);
    assert(out[2] == 3.0f);
    assert(out[3] == 4.5f);

    /* Elementwise min: {-1.5, -2, -3.5, -4}. */
    const psimd_f32 mn = psimd_min_f32(a, b);
    psimd_store_f32(out, mn);
    assert(out[0] == -1.5f);
    assert(out[1] == -2.0f);
    assert(out[2] == -3.5f);
    assert(out[3] == -4.0f);

    /* abs: clear the sign bit. */
    const psimd_f32 ab = psimd_abs_f32(a);
    psimd_store_f32(out, ab);
    assert(out[0] == 1.0f);
    assert(out[1] == 2.0f);
    assert(out[2] == 3.0f);
    assert(out[3] == 4.0f);

    /* neg: flip the sign bit. */
    const psimd_f32 ng = psimd_neg_f32(a);
    psimd_store_f32(out, ng);
    assert(out[0] == -1.0f);
    assert(out[1] ==  2.0f);
    assert(out[2] == -3.0f);
    assert(out[3] ==  4.0f);

    /* qfma: a + b * c. With a=splat(1), b=splat(2), c=splat(3) -> 1 + 6 = 7. */
    const psimd_f32 fa = psimd_splat_f32(1.0f);
    const psimd_f32 fb = psimd_splat_f32(2.0f);
    const psimd_f32 fc = psimd_splat_f32(3.0f);
    const psimd_f32 fm = psimd_qfma_f32(fa, fb, fc);
    psimd_store_f32(out, fm);
    assert(out[0] == 7.0f);
    assert(out[3] == 7.0f);

    /* Integer max/min over signed 32-bit. */
    const psimd_s32 sa = (psimd_s32) { -5,  10, -15,  20 };
    const psimd_s32 sb = (psimd_s32) {  5, -10,  15, -20 };
    const psimd_s32 smax = psimd_max_s32(sa, sb);
    int32_t out_s32[4] __attribute__((aligned(16))) = {0, 0, 0, 0};
    psimd_store_s32(out_s32, smax);
    assert(out_s32[0] ==  5);
    assert(out_s32[1] == 10);
    assert(out_s32[2] == 15);
    assert(out_s32[3] == 20);

    printf("psimd minmax test OK\n");
    return 0;
}
