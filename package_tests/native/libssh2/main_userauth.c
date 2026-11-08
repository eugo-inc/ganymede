/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: userauth; calls libssh2_init(), libssh2_session_init(), libssh2_userauth_authenticated(), libssh2_userauth_password_ex().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libssh2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Dummy password-change callback — never invoked here; only its address is
 * passed through libssh2_userauth_password_ex, proving the link target
 * accepts the LIBSSH2_PASSWD_CHANGEREQ_FUNC signature. */
static LIBSSH2_PASSWD_CHANGEREQ_FUNC(pw_change_cb) {
    (void)session;
    (void)newpw;
    (void)newpw_len;
    (void)abstract;
}

int main(void) {
    assert(libssh2_init(0) == 0);

    LIBSSH2_SESSION *session = libssh2_session_init();
    assert(session != NULL);

    /* On a fresh, unconnected session, the user is not authenticated. The accessor
     * must return 0 (not authenticated) — proves the local-state path works without
     * any transport. */
    assert(libssh2_userauth_authenticated(session) == 0);

    const char *user = "nobody";
    const char *pass = "x";
    int rc = libssh2_userauth_password_ex(session,
                                          user, (unsigned int)strlen(user),
                                          pass, (unsigned int)strlen(pass),
                                          pw_change_cb);
    assert(rc != 0);  /* must fail — no transport */
    assert(libssh2_session_last_errno(session) != LIBSSH2_ERROR_NONE);

    const char *list = libssh2_userauth_list(session, "nobody", 6);
    assert(list == NULL);
    assert(libssh2_session_last_errno(session) != LIBSSH2_ERROR_NONE);

    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 userauth test OK\n");
    return 0;
}
