/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzmq: poll; calls zmq_ctx_new(), zmq_socket(), zmq_bind(), zmq_connect().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zmq.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    void *ctx = zmq_ctx_new();
    assert(ctx != NULL);

    void *a = zmq_socket(ctx, ZMQ_PAIR);
    void *b = zmq_socket(ctx, ZMQ_PAIR);
    assert(a != NULL && b != NULL);

    assert(zmq_bind(a, "inproc://demo-poll") == 0);
    assert(zmq_connect(b, "inproc://demo-poll") == 0);

    zmq_pollitem_t items[1];
    items[0].socket = a;
    items[0].fd = 0;
    items[0].events = ZMQ_POLLIN;
    items[0].revents = 0;

    /* 1. Idle poll — nothing sent yet, expect 0 (timeout) and no POLLIN. */
    int rc = zmq_poll(items, 1, 50 /* ms */);
    assert(rc == 0);
    assert((items[0].revents & ZMQ_POLLIN) == 0);

    /* 2. Send from b, then poll on a — expect 1 ready socket with POLLIN set. */
    const char msg[] = "ping";
    assert(zmq_send(b, msg, sizeof(msg) - 1, 0) == (int)(sizeof(msg) - 1));

    /* Generous timeout — give inproc handshake / scheduling room. */
    items[0].revents = 0;
    rc = zmq_poll(items, 1, 1000 /* ms */);
    assert(rc == 1);
    assert((items[0].revents & ZMQ_POLLIN) != 0);

    /* Drain the message so close is clean. */
    char buf[16] = {0};
    int n = zmq_recv(a, buf, sizeof(buf) - 1, 0);
    assert(n == (int)(sizeof(msg) - 1));
    assert(memcmp(buf, msg, sizeof(msg) - 1) == 0);

    assert(zmq_close(b) == 0);
    assert(zmq_close(a) == 0);
    assert(zmq_ctx_term(ctx) == 0);

    printf("libzmq zmq_poll test OK\n");
    return 0;
}
