/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp2: session; calls nghttp2_session_callbacks_new(), nghttp2_session_client_new(), nghttp2_session_want_read(), nghttp2_submit_settings().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    nghttp2_session_callbacks *callbacks = NULL;
    int rv = nghttp2_session_callbacks_new(&callbacks);
    assert(rv == 0);
    assert(callbacks != NULL);

    int sentinel = 42;
    nghttp2_session *session = NULL;
    rv = nghttp2_session_client_new(&session, callbacks, &sentinel);
    assert(rv == 0);
    assert(session != NULL);

    /* A fresh client always wants to read — it is waiting on the
     * server's SETTINGS / SETTINGS-ACK frames. */
    assert(nghttp2_session_want_read(session) != 0);

    /* Queue a SETTINGS frame (empty IV is a valid no-op SETTINGS). The
     * submit MUST succeed AND it MUST cause want_write to flip true,
     * proving the send-queue machinery is alive. */
    rv = nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, NULL, 0);
    assert(rv == 0);
    assert(nghttp2_session_want_write(session) != 0);

    nghttp2_session_del(session);
    nghttp2_session_callbacks_del(callbacks);

    printf("nghttp2 client-session lifecycle OK\n");
    return 0;
}
