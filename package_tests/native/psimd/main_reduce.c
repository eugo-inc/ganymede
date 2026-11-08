/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for psimd: reduce; calls psimd_load_f32(), psimd_reduce_sum_f32(), psimd_reduce_max_f32(), psimd_reduce_min_f32().
 */

#include <psimd.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    float in[4] __attribute__((aligned(16))) = {1.0f, 2.0f, 3.0f, 4.0f};
    const psimd_f32 v = psimd_load_f32(in);

    const float sum_out = psimd_reduce_sum_f32(v);
    assert(sum_out == 10.0f);

    /* reduce_max: max(1,2,3,4) = 4. Returns a scalar float. */
    const float max_out = psimd_reduce_max_f32(v);
    assert(max_out == 4.0f);

    /* reduce_min: min(1,2,3,4) = 1. Returns a scalar float. */
    const float min_out = psimd_reduce_min_f32(v);
    assert(min_out == 1.0f);

    /* allreduce_sum: 10 broadcast across all 4 lanes. */
    const psimd_f32 allsum = psimd_allreduce_sum_f32(v);
    float all_out[4] __attribute__((aligned(16))) = {0.0f, 0.0f, 0.0f, 0.0f};
    psimd_store_f32(all_out, allsum);
    assert(all_out[0] == 10.0f);
    assert(all_out[1] == 10.0f);
    assert(all_out[2] == 10.0f);
    assert(all_out[3] == 10.0f);

    /* allreduce_max: 4 broadcast across all 4 lanes. */
    const psimd_f32 allmax = psimd_allreduce_max_f32(v);
    psimd_store_f32(all_out, allmax);
    assert(all_out[0] == 4.0f);
    assert(all_out[3] == 4.0f);

    /* allreduce_min on a vector with a clear unique minimum. */
    float neg_in[4] __attribute__((aligned(16))) = {5.0f, -7.0f, 3.0f, 2.0f};
    const psimd_f32 nv = psimd_load_f32(neg_in);
    const psimd_f32 allmin = psimd_allreduce_min_f32(nv);
    psimd_store_f32(all_out, allmin);
    assert(all_out[0] == -7.0f);
    assert(all_out[1] == -7.0f);
    assert(all_out[2] == -7.0f);
    assert(all_out[3] == -7.0f);

    printf("psimd reduce test OK\n");
    return 0;
}
