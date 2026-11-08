/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for psimd: int; calls psimd_splat_s32(), psimd_add_s32(), psimd_sub_s32(), psimd_mul_s32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* 4x int32: splat 7 + splat 3 -> 10; subtract -> 4; mul -> 40. */
    const psimd_s32 a32 = psimd_splat_s32(7);
    const psimd_s32 b32 = psimd_splat_s32(3);
    const psimd_s32 sum32 = psimd_add_s32(a32, b32);
    const psimd_s32 dif32 = psimd_sub_s32(a32, b32);
    const psimd_s32 mul32 = psimd_mul_s32(sum32, dif32);

    int32_t out32[4] __attribute__((aligned(16))) = {0, 0, 0, 0};
    psimd_store_s32(out32, mul32);
    assert(out32[0] == 40);
    assert(out32[1] == 40);
    assert(out32[2] == 40);
    assert(out32[3] == 40);

    /* Sanity-check the intermediate add: store sum and read back. */
    psimd_store_s32(out32, sum32);
    assert(out32[0] == 10);
    assert(out32[3] == 10);

    /* 16x uint8: load from buffer, add splat(1) wrapping, store back. */
    uint8_t in8[16] __attribute__((aligned(16))) = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 254
    };
    const psimd_u8 v8 = psimd_load_u8(in8);
    const psimd_u8 one8 = psimd_splat_u8(1);
    const psimd_u8 inc8 = psimd_add_u8(v8, one8);

    uint8_t out8[16] __attribute__((aligned(16))) = {0};
    psimd_store_u8(out8, inc8);
    assert(out8[0] == 1);
    assert(out8[7] == 8);
    assert(out8[14] == 15);
    /* Wrap: 254 + 1 = 255. */
    assert(out8[15] == 255);

    printf("psimd int test OK\n");
    return 0;
}
