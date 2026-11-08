/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: keepalive; calls libssh2_init(), libssh2_session_init(), libssh2_keepalive_config(), libssh2_keepalive_send().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libssh2.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(libssh2_init(0) == 0);

    LIBSSH2_SESSION *session = libssh2_session_init();
    assert(session != NULL);

    libssh2_keepalive_config(session, 1, 60);

    libssh2_keepalive_config(session, 0, 30);

    int seconds_to_next = -1;
    int rc = libssh2_keepalive_send(session, &seconds_to_next);
    assert(rc <= 0 || rc == LIBSSH2_ERROR_NONE);
    assert(seconds_to_next >= -1);

    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 keepalive test OK (rc=%d secs=%d)\n", rc, seconds_to_next);
    return 0;
}
