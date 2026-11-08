/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pthreadpool: core API; calls pthreadpool_create(), pthreadpool_get_threads_count(),
 * pthreadpool_parallelize_1d(), pthreadpool_destroy().
 */

#include <pthreadpool.h>
#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

#define WORK_ITEMS 64

static void increment_task(void *context, size_t index) {
    atomic_int *counter = (atomic_int *)context;
    (void)index;
    atomic_fetch_add(counter, 1);
}

int main(void) {
    /* 0 threads => library picks a default based on detected cores. */
    pthreadpool_t pool = pthreadpool_create(0);
    assert(pool != NULL);

    /* threads_count must be >= 1 on any system pthreadpool supports. */
    size_t threads = pthreadpool_get_threads_count(pool);
    assert(threads >= 1);

    /* parallelize_1d must invoke the callback exactly WORK_ITEMS times. */
    atomic_int counter = 0;
    pthreadpool_parallelize_1d(pool, increment_task, &counter, WORK_ITEMS, 0);
    assert(atomic_load(&counter) == WORK_ITEMS);

    pthreadpool_destroy(pool);

    printf("pthreadpool test OK: threads=%zu items=%d\n", threads, WORK_ITEMS);
    return 0;
}
