/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libuv: core API; calls uv_timer_stop(), uv_close(), uv_default_loop(), uv_timer_init().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <uv.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int fired = 0;

static void on_timer(uv_timer_t *handle) {
    fired += 1;
    uv_timer_stop(handle);
    uv_close((uv_handle_t *)handle, NULL);
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();
    assert(loop != NULL);

    uv_timer_t timer;
    assert(uv_timer_init(loop, &timer) == 0);
    assert(uv_timer_start(&timer, on_timer, 1, 0) == 0);

    assert(uv_run(loop, UV_RUN_DEFAULT) == 0);
    assert(fired == 1);

    assert(uv_loop_close(loop) == 0);

    const char *v = uv_version_string();
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("libuv test OK: %s\n", v);
    return 0;
}
