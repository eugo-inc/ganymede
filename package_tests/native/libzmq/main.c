/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzmq: core API; calls zmq_version(), zmq_ctx_new(), zmq_socket(), zmq_bind().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zmq.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int major = -1, minor = -1, patch = -1;
    zmq_version(&major, &minor, &patch);
    assert(major == 4);
    assert(minor >= 0 && patch >= 0);

    void *ctx = zmq_ctx_new();
    assert(ctx != NULL);

    void *server = zmq_socket(ctx, ZMQ_PAIR);
    void *client = zmq_socket(ctx, ZMQ_PAIR);
    assert(server != NULL && client != NULL);

    assert(zmq_bind(server, "inproc://demo-test") == 0);
    assert(zmq_connect(client, "inproc://demo-test") == 0);

    const char msg[] = "hello";
    assert(zmq_send(client, msg, sizeof(msg) - 1, 0) == (int)(sizeof(msg) - 1));

    char buf[16] = {0};
    int n = zmq_recv(server, buf, sizeof(buf) - 1, 0);
    assert(n == (int)(sizeof(msg) - 1));
    assert(memcmp(buf, msg, sizeof(msg) - 1) == 0);

    assert(zmq_close(client) == 0);
    assert(zmq_close(server) == 0);
    assert(zmq_ctx_term(ctx) == 0);

    printf("libzmq test OK: %d.%d.%d\n", major, minor, patch);
    return 0;
}
