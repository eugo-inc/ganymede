/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzmq: gssapi; calls zmq_has(), zmq_ctx_new(), zmq_socket(), zmq_setsockopt().
 */

#include <zmq.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("zmq_has(\"curve\") = %d, zmq_has(\"gssapi\") = %d\n", zmq_has("curve"), zmq_has("gssapi"));
    assert(zmq_has("curve") == 1);
    assert(zmq_has("gssapi") == 1);

    void *ctx = zmq_ctx_new();
    assert(ctx != NULL);
    void *s = zmq_socket(ctx, ZMQ_REP);
    assert(s != NULL);

    int as_server = 1;
    int rc = zmq_setsockopt(s, ZMQ_GSSAPI_SERVER, &as_server, sizeof(as_server));
    printf("zmq_setsockopt(ZMQ_GSSAPI_SERVER) = %d (errno %d: %s)\n", rc, rc == 0 ? 0 : errno, rc == 0 ? "-" : strerror(errno));
    assert(rc == 0);

    int mechanism = -1;
    size_t mechanism_len = sizeof(mechanism);
    assert(zmq_getsockopt(s, ZMQ_MECHANISM, &mechanism, &mechanism_len) == 0);
    assert(mechanism == ZMQ_GSSAPI);

    assert(zmq_close(s) == 0);
    assert(zmq_ctx_term(ctx) == 0);
    printf("libzmq GSSAPI test OK\n");
    return 0;
}
