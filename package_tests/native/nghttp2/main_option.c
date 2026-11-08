/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp2: option; calls nghttp2_option_new(), nghttp2_option_set_no_auto_window_update(), nghttp2_option_set_peer_max_concurrent_streams(), nghttp2_option_set_no_recv_client_magic().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    nghttp2_option *option = NULL;
    int rv = nghttp2_option_new(&option);
    assert(rv == 0);
    assert(option != NULL);

    nghttp2_option_set_no_auto_window_update(option, 1);
    nghttp2_option_set_peer_max_concurrent_streams(option, 100);
    nghttp2_option_set_no_recv_client_magic(option, 1);

    nghttp2_session_callbacks *callbacks = NULL;
    rv = nghttp2_session_callbacks_new(&callbacks);
    assert(rv == 0);
    assert(callbacks != NULL);

    nghttp2_session *session = NULL;
    rv = nghttp2_session_client_new2(&session, callbacks, NULL, option);
    assert(rv == 0);
    assert(session != NULL);

    /* Sanity: the session is alive and acts like a client (wants to read
     * the server's preface). */
    assert(nghttp2_session_want_read(session) != 0);

    nghttp2_session_del(session);
    nghttp2_session_callbacks_del(callbacks);
    nghttp2_option_del(option);

    printf("nghttp2 option API OK: session accepted populated option\n");
    return 0;
}
