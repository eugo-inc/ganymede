/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pthreadpool: 3d; calls pthreadpool_create(), pthreadpool_parallelize_3d().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define RANGE_I 3
#define RANGE_J 4
#define RANGE_K 5
#define GRID_CELLS (RANGE_I * RANGE_J * RANGE_K)

struct grid3d_ctx {
    atomic_int visits[GRID_CELLS];
};

static void mark_cell_3d(void *context, size_t i, size_t j, size_t k) {
    struct grid3d_ctx *ctx = (struct grid3d_ctx *)context;
    /* Bounds checks catch a scheduler that overshoots any axis. */
    assert(i < RANGE_I);
    assert(j < RANGE_J);
    assert(k < RANGE_K);
    size_t idx = (i * RANGE_J + j) * RANGE_K + k;
    atomic_fetch_add(&ctx->visits[idx], 1);
}

int main(void) {
    pthreadpool_t pool = pthreadpool_create(0);
    assert(pool != NULL);

    struct grid3d_ctx ctx;
    for (size_t n = 0; n < GRID_CELLS; n++) {
        atomic_store(&ctx.visits[n], 0);
    }

    pthreadpool_parallelize_3d(pool, mark_cell_3d, &ctx,
                               RANGE_I, RANGE_J, RANGE_K, 0);

    /* Every cell must have been visited exactly once. */
    int total = 0;
    for (size_t n = 0; n < GRID_CELLS; n++) {
        int v = atomic_load(&ctx.visits[n]);
        assert(v == 1);
        total += v;
    }
    assert(total == GRID_CELLS);

    pthreadpool_destroy(pool);

    printf("pthreadpool 3D test OK: grid=%dx%dx%d cells=%d\n",
           RANGE_I, RANGE_J, RANGE_K, GRID_CELLS);
    return 0;
}
