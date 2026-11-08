/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libuv: work; calls uv_thread_self(), uv_async_send(), uv_thread_equal(), uv_close().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <uv.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static uv_thread_t loop_thread_id;
static uv_thread_t work_thread_id;
static int work_ran = 0;
static int after_work_ran = 0;
static int async_fired = 0;

static uv_async_t async_handle;

static void work_cb(uv_work_t *req) {
    work_thread_id = uv_thread_self();
    work_ran += 1;
    /* signal the loop from this worker thread */
    uv_async_send(&async_handle);
}

static void after_work_cb(uv_work_t *req, int status) {
    assert(status == 0);
    after_work_ran += 1;
    /* must run on the loop thread (i.e. NOT the worker thread). */
    uv_thread_t self = uv_thread_self();
    assert(uv_thread_equal(&self, &work_thread_id) == 0);
}

static void async_cb(uv_async_t *handle) {
    async_fired += 1;
    uv_close((uv_handle_t *)handle, NULL);
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();
    assert(loop != NULL);

    loop_thread_id = uv_thread_self();

    assert(uv_async_init(loop, &async_handle, async_cb) == 0);

    uv_work_t work_req;
    assert(uv_queue_work(loop, &work_req, work_cb, after_work_cb) == 0);

    assert(uv_run(loop, UV_RUN_DEFAULT) == 0);

    assert(work_ran == 1);
    assert(after_work_ran == 1);
    assert(async_fired == 1);
    /* The work callback must have run on a thread distinct from the loop thread. */
    assert(uv_thread_equal(&loop_thread_id, &work_thread_id) == 0);

    assert(uv_loop_close(loop) == 0);

    printf("libuv work+async test OK\n");
    return 0;
}
