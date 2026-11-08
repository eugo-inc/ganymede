/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp2: server; calls nghttp2_session_callbacks_new(), nghttp2_session_server_new(), nghttp2_session_want_read(), nghttp2_submit_settings().
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

    nghttp2_session *session = NULL;
    rv = nghttp2_session_server_new(&session, callbacks, NULL);
    assert(rv == 0);
    assert(session != NULL);

    /* A fresh server is always waiting on the client's connection preface
     * + initial SETTINGS — it must want to read. */
    assert(nghttp2_session_want_read(session) != 0);

    /* Submit the server's own SETTINGS frame. Empty IV is a valid no-op
     * SETTINGS. Submit must succeed AND want_write must flip true. */
    rv = nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, NULL, 0);
    assert(rv == 0);
    assert(nghttp2_session_want_write(session) != 0);

    uint32_t mcs = nghttp2_session_get_local_settings(
        session, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
    assert(mcs == 0xffffffffu);

    nghttp2_session_del(session);
    nghttp2_session_callbacks_del(callbacks);

    printf("nghttp2 server-session lifecycle OK: default MAX_CONCURRENT_STREAMS=0x%x\n", mcs);
    return 0;
}
