/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for psimd: cvt div; calls psimd_cvt_s32_f32(), psimd_store_f32(), psimd_load_f32(), psimd_div_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* cvt_s32_f32: convert int32 {1, 2, 3, 4} -> float {1.0, 2.0, 3.0, 4.0}. */
    const psimd_s32 si = (psimd_s32) { 1, 2, 3, 4 };
    const psimd_f32 fi = psimd_cvt_s32_f32(si);
    float out_f[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(out_f, fi);
    assert(out_f[0] == 1.0f);
    assert(out_f[1] == 2.0f);
    assert(out_f[2] == 3.0f);
    assert(out_f[3] == 4.0f);

    /* cvt of negative int32. */
    const psimd_s32 neg = (psimd_s32) { -1, -2, -3, -4 };
    const psimd_f32 negf = psimd_cvt_s32_f32(neg);
    psimd_store_f32(out_f, negf);
    assert(out_f[0] == -1.0f);
    assert(out_f[3] == -4.0f);

    /* div_f32: elementwise {8,9,10,12} / {2,3,5,4} = {4,3,2,3}. */
    float num_buf[4] __attribute__((aligned(16))) = { 8.0f, 9.0f, 10.0f, 12.0f };
    float den_buf[4] __attribute__((aligned(16))) = { 2.0f, 3.0f,  5.0f,  4.0f };
    const psimd_f32 num = psimd_load_f32(num_buf);
    const psimd_f32 den = psimd_load_f32(den_buf);
    const psimd_f32 quot = psimd_div_f32(num, den);
    psimd_store_f32(out_f, quot);
    assert(out_f[0] == 4.0f);
    assert(out_f[1] == 3.0f);
    assert(out_f[2] == 2.0f);
    assert(out_f[3] == 3.0f);

    /* load_splat for int32: broadcast a scalar int32 across all 4 lanes. */
    const int32_t scalar32 = 42;
    const psimd_s32 splatted32 = psimd_load_splat_s32(&scalar32);
    int32_t out_s32[4] __attribute__((aligned(16))) = {0, 0, 0, 0};
    psimd_store_s32(out_s32, splatted32);
    assert(out_s32[0] == 42);
    assert(out_s32[1] == 42);
    assert(out_s32[2] == 42);
    assert(out_s32[3] == 42);

    printf("psimd cvt/div test OK\n");
    return 0;
}
