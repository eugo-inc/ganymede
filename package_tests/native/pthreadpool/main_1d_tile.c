/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pthreadpool: 1d tile; calls pthreadpool_create(), pthreadpool_parallelize_1d_tile_1d().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define RANGE 23
#define TILE 5

struct tile1d_ctx {
    atomic_int visits[RANGE];
    atomic_int tile_invocations;
};

static size_t min_size(size_t a, size_t b) {
    return a < b ? a : b;
}

static void mark_tile(void *context, size_t offset, size_t tile) {
    struct tile1d_ctx *ctx = (struct tile1d_ctx *)context;
    atomic_fetch_add(&ctx->tile_invocations, 1);

    /* Offsets must be TILE-aligned. */
    assert(offset % TILE == 0);
    /* Tile size must equal min(range - offset, TILE). */
    size_t expected = min_size((size_t)RANGE - offset, (size_t)TILE);
    assert(tile == expected);
    assert(offset + tile <= (size_t)RANGE);

    for (size_t i = offset; i < offset + tile; i++) {
        atomic_fetch_add(&ctx->visits[i], 1);
    }
}

int main(void) {
    pthreadpool_t pool = pthreadpool_create(0);
    assert(pool != NULL);

    struct tile1d_ctx ctx;
    for (size_t k = 0; k < RANGE; k++) {
        atomic_store(&ctx.visits[k], 0);
    }
    atomic_store(&ctx.tile_invocations, 0);

    pthreadpool_parallelize_1d_tile_1d(pool, mark_tile, &ctx, RANGE, TILE, 0);

    /* Every index visited exactly once. */
    int total = 0;
    for (size_t k = 0; k < RANGE; k++) {
        int v = atomic_load(&ctx.visits[k]);
        assert(v == 1);
        total += v;
    }
    assert(total == RANGE);

    /* Tile invocation count: ceil(range / tile). */
    int expected_invocations = (int)((RANGE + TILE - 1) / TILE);
    assert(atomic_load(&ctx.tile_invocations) == expected_invocations);

    pthreadpool_destroy(pool);

    printf("pthreadpool 1D-tile test OK: range=%d tile=%d invocations=%d\n",
           RANGE, TILE, expected_invocations);
    return 0;
}
