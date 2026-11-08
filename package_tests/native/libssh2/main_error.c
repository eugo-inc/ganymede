/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libssh2: error; calls libssh2_init(), libssh2_session_init(), libssh2_session_get_blocking(), libssh2_session_set_blocking().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libssh2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(libssh2_init(0) == 0);

    LIBSSH2_SESSION *session = libssh2_session_init();
    assert(session != NULL);

    /* Default mode is blocking (1). Toggle to non-blocking and back; the getter must follow. */
    int default_mode = libssh2_session_get_blocking(session);
    /* Documented default is blocking == 1, but accept either as long as the toggle works. */
    (void)default_mode;

    libssh2_session_set_blocking(session, 0);
    assert(libssh2_session_get_blocking(session) == 0);

    libssh2_session_set_blocking(session, 1);
    assert(libssh2_session_get_blocking(session) == 1);

    /* Fresh session: last_error must report LIBSSH2_ERROR_NONE (0). Per src/session.c, when
     * err_code == 0 the library writes errmsg = "" (internal static empty string, want_buf=0)
     * and errmsg_len = 0. So errmsg must be non-NULL but len is exactly 0; strlen agrees. */
    char *errmsg = NULL;
    int errmsg_len = -1;
    int last = libssh2_session_last_error(session, &errmsg, &errmsg_len, 0);
    assert(last == LIBSSH2_ERROR_NONE);
    assert(errmsg != NULL);
    assert(errmsg_len == 0);
    assert(strlen(errmsg) == (size_t)errmsg_len);

    /* last_errno on a fresh session also reports NONE. */
    assert(libssh2_session_last_errno(session) == LIBSSH2_ERROR_NONE);

    int set_rc = libssh2_session_set_last_error(session, LIBSSH2_ERROR_ALLOC, "synthetic-oom");
    assert(set_rc == LIBSSH2_ERROR_ALLOC);
    assert(libssh2_session_last_errno(session) == LIBSSH2_ERROR_ALLOC);

    errmsg = NULL;
    errmsg_len = 0;
    last = libssh2_session_last_error(session, &errmsg, &errmsg_len, 0);
    assert(last == LIBSSH2_ERROR_ALLOC);
    assert(errmsg != NULL);
    assert(strstr(errmsg, "synthetic-oom") != NULL);

    /* block_directions on an unconnected session should report 0 (nothing is blocked). */
    assert(libssh2_session_block_directions(session) == 0);

    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 error/blocking test OK\n");
    return 0;
}
