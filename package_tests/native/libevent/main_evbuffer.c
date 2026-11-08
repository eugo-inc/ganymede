/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libevent: evbuffer; calls evbuffer_new(), evbuffer_get_length(), evbuffer_add(), evbuffer_remove().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <event2/buffer.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Exercises libevent's evbuffer data-pipeline API: separate compilation unit
 * from event.h, lives in libevent_core. Failure here means evbuffer internals
 * (add/remove/length) are broken. */

int main(void) {
    struct evbuffer *buf = evbuffer_new();
    assert(buf != NULL);
    assert(evbuffer_get_length(buf) == 0);

    const char *msg = "hello demo libevent";
    int rc = evbuffer_add(buf, msg, strlen(msg));
    assert(rc == 0);
    assert(evbuffer_get_length(buf) == strlen(msg));

    char out[64] = {0};
    ev_ssize_t n = evbuffer_remove(buf, out, sizeof(out) - 1);
    assert(n == (ev_ssize_t)strlen(msg));
    assert(strcmp(out, msg) == 0);
    assert(evbuffer_get_length(buf) == 0);

    /* Exercise add_printf — used by HTTP / RPC consumers. */
    rc = evbuffer_add_printf(buf, "n=%d s=%s", 42, "ok");
    assert(rc > 0);
    assert(evbuffer_get_length(buf) == (size_t)rc);

    evbuffer_free(buf);
    printf("libevent evbuffer test OK\n");
    return 0;
}
