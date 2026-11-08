/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pthreadpool: with thread; calls pthreadpool_create(), pthreadpool_parallelize_1d_with_thread(),
 * pthreadpool_get_threads_count().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define POOL_THREADS 4
#define WORK_ITEMS 64

struct thread_ctx {
    atomic_int item_visits[WORK_ITEMS];
    atomic_int max_thread_index_plus_one;
};

static void mark_with_thread(void *context, size_t thread_index, size_t item) {
    struct thread_ctx *ctx = (struct thread_ctx *)context;
    assert(item < WORK_ITEMS);
    atomic_fetch_add(&ctx->item_visits[item], 1);

    /* Track the largest thread_index + 1 seen via a monotonic max. */
    int candidate = (int)thread_index + 1;
    int prev = atomic_load(&ctx->max_thread_index_plus_one);
    while (candidate > prev) {
        if (atomic_compare_exchange_weak(
                &ctx->max_thread_index_plus_one, &prev, candidate)) {
            break;
        }
    }
}

int main(void) {
    pthreadpool_t pool = pthreadpool_create(POOL_THREADS);
    assert(pool != NULL);
    size_t threads = pthreadpool_get_threads_count(pool);
    assert(threads >= 1);

    struct thread_ctx ctx;
    for (size_t k = 0; k < WORK_ITEMS; k++) {
        atomic_store(&ctx.item_visits[k], 0);
    }
    atomic_store(&ctx.max_thread_index_plus_one, 0);

    pthreadpool_parallelize_1d_with_thread(
        pool, mark_with_thread, &ctx, WORK_ITEMS, 0);

    /* Every item must have been visited exactly once. */
    for (size_t k = 0; k < WORK_ITEMS; k++) {
        assert(atomic_load(&ctx.item_visits[k]) == 1);
    }

    /* Every observed thread_index must be in [0, threads). */
    int max_plus_one = atomic_load(&ctx.max_thread_index_plus_one);
    assert(max_plus_one >= 1);
    assert((size_t)max_plus_one <= threads);

    pthreadpool_destroy(pool);

    printf("pthreadpool with_thread test OK: items=%d threads=%zu max_tid+1=%d\n",
           WORK_ITEMS, threads, max_plus_one);
    return 0;
}
