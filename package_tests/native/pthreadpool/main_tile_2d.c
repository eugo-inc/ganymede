/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pthreadpool: tile 2d; calls pthreadpool_create(), pthreadpool_parallelize_2d_tile_1d().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define RANGE_I 5
#define RANGE_J 17
#define TILE_J 4
#define GRID_CELLS (RANGE_I * RANGE_J)

struct tile_ctx {
    atomic_int visits[GRID_CELLS];
    atomic_int tile_invocations;
};

static size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

static void mark_tile(void *context, size_t i, size_t j_start, size_t tile) {
    struct tile_ctx *ctx = (struct tile_ctx *)context;
    atomic_fetch_add(&ctx->tile_invocations, 1);

    /* Tiles must start on tile_j-aligned boundaries. */
    assert(j_start % TILE_J == 0);
    /* Tile size must equal min(range_j - j_start, tile_j). */
    size_t expected = min_size((size_t)RANGE_J - j_start, (size_t)TILE_J);
    assert(tile == expected);
    assert(j_start + tile <= (size_t)RANGE_J);
    assert(i < (size_t)RANGE_I);

    for (size_t j = j_start; j < j_start + tile; j++) {
        atomic_fetch_add(&ctx->visits[i * RANGE_J + j], 1);
    }
}

int main(void) {
    pthreadpool_t pool = pthreadpool_create(0);
    assert(pool != NULL);

    struct tile_ctx ctx;
    for (size_t k = 0; k < GRID_CELLS; k++) {
        atomic_store(&ctx.visits[k], 0);
    }
    atomic_store(&ctx.tile_invocations, 0);

    pthreadpool_parallelize_2d_tile_1d(pool, mark_tile, &ctx,
                                       RANGE_I, RANGE_J, TILE_J, 0);

    /* Every cell visited exactly once. */
    int total = 0;
    for (size_t k = 0; k < GRID_CELLS; k++) {
        int v = atomic_load(&ctx.visits[k]);
        assert(v == 1);
        total += v;
    }
    assert(total == GRID_CELLS);

    /* Tile invocation count: ceil(range_j / tile_j) * range_i. */
    int expected_invocations =
        (int)(((RANGE_J + TILE_J - 1) / TILE_J) * RANGE_I);
    assert(atomic_load(&ctx.tile_invocations) == expected_invocations);

    pthreadpool_destroy(pool);

    printf("pthreadpool 2D-tile-1D test OK: grid=%dx%d tile_j=%d invocations=%d\n",
           RANGE_I, RANGE_J, TILE_J, expected_invocations);
    return 0;
}
