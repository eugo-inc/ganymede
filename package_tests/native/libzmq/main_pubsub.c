/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzmq: pubsub; calls zmq_ctx_new(), zmq_socket(), zmq_setsockopt(), zmq_bind().
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

    void *pub = zmq_socket(ctx, ZMQ_PUB);
    void *sub = zmq_socket(ctx, ZMQ_SUB);
    assert(pub != NULL && sub != NULL);

    const char topic[] = "demo.";
    assert(zmq_setsockopt(sub, ZMQ_SUBSCRIBE, topic, sizeof(topic) - 1) == 0);

    /* 500 ms recv timeout — covers slow-joiner latency without hanging on a bug. */
    int rcvtimeo = 500;
    assert(zmq_setsockopt(sub, ZMQ_RCVTIMEO, &rcvtimeo, sizeof(rcvtimeo)) == 0);

    assert(zmq_bind(pub, "inproc://demo-pubsub") == 0);
    assert(zmq_connect(sub, "inproc://demo-pubsub") == 0);

    /* Publish a few messages — both matching and non-matching the subscription. */
    const char msg_match[] = "demo.hello";
    const char msg_skip[]  = "other.skip";

    /* Retry the matching publish until SUB receives it (inproc PUB/SUB still has
     * a brief connection handshake; retrying defeats slow-joiner without sleeps). */
    char buf[64] = {0};
    int n = -1;
    for (int i = 0; i < 20; ++i) {
        assert(zmq_send(pub, msg_skip,  sizeof(msg_skip)  - 1, 0) == (int)(sizeof(msg_skip)  - 1));
        assert(zmq_send(pub, msg_match, sizeof(msg_match) - 1, 0) == (int)(sizeof(msg_match) - 1));
        n = zmq_recv(sub, buf, sizeof(buf) - 1, 0);
        if (n >= 0) break;
    }
    assert(n == (int)(sizeof(msg_match) - 1));
    assert(memcmp(buf, msg_match, sizeof(msg_match) - 1) == 0);

    /* The non-matching message must NOT arrive — drain with a short-timeout recv. */
    char buf2[64] = {0};
    int n2 = zmq_recv(sub, buf2, sizeof(buf2) - 1, 0);
    /* Either timeout (n2 == -1, errno == EAGAIN) OR another matching message arrived
     * from a retry iteration. The non-matching topic must never appear. */
    if (n2 >= 0) {
        assert(memcmp(buf2, "other.", 6) != 0);
    }

    assert(zmq_close(sub) == 0);
    assert(zmq_close(pub) == 0);
    assert(zmq_ctx_term(ctx) == 0);

    printf("libzmq PUB/SUB topic-filter test OK\n");
    return 0;
}
