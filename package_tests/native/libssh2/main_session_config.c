/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: session config; calls libssh2_init(), libssh2_session_init(), libssh2_session_banner_set(), libssh2_session_set_timeout().
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

    /* banner_set: client banner is purely a write-then-send config knob. Setting a
     * syntactically valid banner must succeed. */
    int banner_rc = libssh2_session_banner_set(session, "SSH-2.0-demo_native_test");
    assert(banner_rc == 0);

    libssh2_session_set_timeout(session, 12345);
    assert(libssh2_session_get_timeout(session) == 12345);

    /* 0 means "no timeout" — also a valid value. */
    libssh2_session_set_timeout(session, 0);
    assert(libssh2_session_get_timeout(session) == 0);

    /* session_flag: documented to return 0 on success for a known flag.
     * LIBSSH2_FLAG_COMPRESS is one of the few flags safe to toggle before connect. */
    int flag_rc = libssh2_session_flag(session, LIBSSH2_FLAG_COMPRESS, 1);
    assert(flag_rc == 0);
    flag_rc = libssh2_session_flag(session, LIBSSH2_FLAG_COMPRESS, 0);
    assert(flag_rc == 0);

    /* libssh2_session_abstract returns &abstract pointer slot on the session. Must
     * be non-NULL (it's the address of an internal void* field). */
    void **abstract_slot = libssh2_session_abstract(session);
    assert(abstract_slot != NULL);

    *abstract_slot = (void *)0xdeadbeef;
    abstract_slot = libssh2_session_abstract(session);
    assert(*abstract_slot == (void *)0xdeadbeef);
    *abstract_slot = NULL;

    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 session_config test OK\n");
    return 0;
}
