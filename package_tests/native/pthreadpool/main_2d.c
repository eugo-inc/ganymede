/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pthreadpool: 2d; calls pthreadpool_create(), pthreadpool_parallelize_2d().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define RANGE_I 7
#define RANGE_J 11
#define GRID_CELLS (RANGE_I * RANGE_J)

struct grid_ctx {
    atomic_int visits[GRID_CELLS];
};

static void mark_cell(void *context, size_t i, size_t j) {
    struct grid_ctx *ctx = (struct grid_ctx *)context;
    /* Asserting bounds here catches a scheduler that overshoots the range. */
    assert(i < RANGE_I);
    assert(j < RANGE_J);
    atomic_fetch_add(&ctx->visits[i * RANGE_J + j], 1);
}

int main(void) {
    pthreadpool_t pool = pthreadpool_create(0);
    assert(pool != NULL);

    struct grid_ctx ctx;
    for (size_t k = 0; k < GRID_CELLS; k++) {
        atomic_store(&ctx.visits[k], 0);
    }

    pthreadpool_parallelize_2d(pool, mark_cell, &ctx, RANGE_I, RANGE_J, 0);

    /* Every cell must have been visited exactly once. */
    int total = 0;
    for (size_t k = 0; k < GRID_CELLS; k++) {
        int v = atomic_load(&ctx.visits[k]);
        assert(v == 1);
        total += v;
    }
    assert(total == GRID_CELLS);

    pthreadpool_destroy(pool);

    printf("pthreadpool 2D test OK: grid=%dx%d cells=%d\n",
           RANGE_I, RANGE_J, GRID_CELLS);
    return 0;
}
