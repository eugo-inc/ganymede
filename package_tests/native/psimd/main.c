/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for psimd: core API; calls psimd_splat_f32(), psimd_add_f32(), psimd_store_f32(), psimd_load_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const psimd_f32 a = psimd_splat_f32(2.0f);
    const psimd_f32 b = psimd_splat_f32(3.0f);
    const psimd_f32 sum = psimd_add_f32(a, b);

    float out[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(out, sum);
    assert(out[0] == 5.0f);
    assert(out[1] == 5.0f);
    assert(out[2] == 5.0f);
    assert(out[3] == 5.0f);

    const psimd_f32 reloaded = psimd_load_f32(out);
    const psimd_f32 doubled = psimd_add_f32(reloaded, reloaded);
    psimd_store_f32(out, doubled);
    assert(out[0] == 10.0f);
    assert(out[3] == 10.0f);

    printf("psimd test OK\n");
    return 0;
}
