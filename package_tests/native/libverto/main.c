/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libverto: core API; calls verto_break(), verto_default(), verto_add_timeout(), verto_free().
 */

#include <stdio.h>
#include <verto.h>

static int timer_fired = 0;

static void on_timeout(verto_ctx *ctx, verto_ev *ev) {
    (void)ev;
    timer_fired = 1;
    verto_break(ctx);
}

int main(void) {
    verto_ctx *ctx = verto_default(NULL, VERTO_EV_TYPE_IO | VERTO_EV_TYPE_TIMEOUT);
    if (ctx == NULL) {
        fprintf(stderr, "LIBVERTO-CHECK: FAIL: verto_default found no event-loop module (is libverto-libev.so.1 "
                        "installed next to libverto.so.1, and does it load?)\n");
        return 1;
    }
    verto_ev *ev = verto_add_timeout(ctx, VERTO_EV_FLAG_NONE, on_timeout, 1);
    if (ev == NULL) {
        fprintf(stderr, "LIBVERTO-CHECK: FAIL: verto_add_timeout returned NULL\n");
        verto_free(ctx);
        return 1;
    }
    verto_run(ctx);
    verto_ev_type types = verto_get_supported_types(ctx);
    verto_free(ctx);
    if (!timer_fired) {
        fprintf(stderr, "LIBVERTO-CHECK: FAIL: the loop returned without running the 1 ms timer\n");
        return 1;
    }
    printf("LIBVERTO-CHECK: PASS (supported event types 0x%x)\n", (unsigned)types);
    return 0;
}
