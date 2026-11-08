/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for psimd: shuffle; calls psimd_load_f32(), psimd_reverse_f32(), psimd_splat0_f32(), psimd_store_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    float in[4] __attribute__((aligned(16))) = {1.0f, 2.0f, 3.0f, 4.0f};
    const psimd_f32 v = psimd_load_f32(in);

    /* Reverse: {1,2,3,4} -> {4,3,2,1}. */
    const psimd_f32 rev = psimd_reverse_f32(v);
    float out[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(out, rev);
    assert(out[0] == 4.0f);
    assert(out[1] == 3.0f);
    assert(out[2] == 2.0f);
    assert(out[3] == 1.0f);

    const psimd_f32 b0 = psimd_splat0_f32(v);
    psimd_store_f32(out, b0);
    assert(out[0] == 1.0f);
    assert(out[1] == 1.0f);
    assert(out[2] == 1.0f);
    assert(out[3] == 1.0f);

    const psimd_f32 b3 = psimd_splat3_f32(v);
    psimd_store_f32(out, b3);
    assert(out[0] == 4.0f);
    assert(out[3] == 4.0f);

    /* load_splat: read scalar 5.0 and splat across 4 lanes. */
    const float scalar = 5.0f;
    const psimd_f32 splatted = psimd_load_splat_f32(&scalar);
    psimd_store_f32(out, splatted);
    assert(out[0] == 5.0f);
    assert(out[1] == 5.0f);
    assert(out[2] == 5.0f);
    assert(out[3] == 5.0f);

    /* Zero-init: produces {0,0,0,0}. */
    const psimd_f32 z = psimd_zero_f32();
    psimd_store_f32(out, z);
    assert(out[0] == 0.0f);
    assert(out[3] == 0.0f);

    printf("psimd shuffle test OK\n");
    return 0;
}
