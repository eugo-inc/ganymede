/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzmq: sockopt; calls zmq_ctx_new(), zmq_socket(), zmq_setsockopt(), zmq_getsockopt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zmq.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    void *ctx = zmq_ctx_new();
    assert(ctx != NULL);

    void *s = zmq_socket(ctx, ZMQ_PAIR);
    assert(s != NULL);

    /* 1. ZMQ_LINGER — int. Set to 0, read back 0. */
    int linger_set = 0;
    assert(zmq_setsockopt(s, ZMQ_LINGER, &linger_set, sizeof(linger_set)) == 0);

    int linger_get = -42;
    size_t linger_len = sizeof(linger_get);
    assert(zmq_getsockopt(s, ZMQ_LINGER, &linger_get, &linger_len) == 0);
    assert(linger_len == sizeof(int));
    assert(linger_get == 0);

    /* 2. ZMQ_SNDHWM — int. Set to 5000, read back 5000. */
    int hwm_set = 5000;
    assert(zmq_setsockopt(s, ZMQ_SNDHWM, &hwm_set, sizeof(hwm_set)) == 0);

    int hwm_get = -1;
    size_t hwm_len = sizeof(hwm_get);
    assert(zmq_getsockopt(s, ZMQ_SNDHWM, &hwm_get, &hwm_len) == 0);
    assert(hwm_get == 5000);

    /* 3. ZMQ_RCVTIMEO: set 50 ms, read it back; a recv with no peer must time out with EAGAIN. */
    int timeo_set = 50;
    assert(zmq_setsockopt(s, ZMQ_RCVTIMEO, &timeo_set, sizeof(timeo_set)) == 0);

    int timeo_get = -1;
    size_t timeo_len = sizeof(timeo_get);
    assert(zmq_getsockopt(s, ZMQ_RCVTIMEO, &timeo_get, &timeo_len) == 0);
    assert(timeo_get == 50);

    /* Bind to an inproc endpoint with no peer, then try to recv. Must time out. */
    assert(zmq_bind(s, "inproc://demo-sockopt") == 0);

    char buf[16];
    int n = zmq_recv(s, buf, sizeof(buf), 0);
    assert(n == -1);
    assert(errno == EAGAIN);

    /* 4. ZMQ_TYPE reads back ZMQ_PAIR. */
    int type_get = -1;
    size_t type_len = sizeof(type_get);
    assert(zmq_getsockopt(s, ZMQ_TYPE, &type_get, &type_len) == 0);
    assert(type_get == ZMQ_PAIR);

    assert(zmq_close(s) == 0);
    assert(zmq_ctx_term(ctx) == 0);

    printf("libzmq sockopt test OK\n");
    return 0;
}
