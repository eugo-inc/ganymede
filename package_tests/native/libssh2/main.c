/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: core API; calls libssh2_init(), libssh2_session_init(), libssh2_session_free(), libssh2_exit().
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

    libssh2_session_free(session);
    libssh2_exit();

    /* libssh2_version(0) returns the version string unconditionally. Must contain a "." separator
     * (e.g. "1.11.0"). Catches a stub / mis-linked .so. */
    const char *v = libssh2_version(0);
    assert(v != NULL);
    assert(strchr(v, '.') != NULL);

    /* libssh2_version with a too-high required version returns NULL — proves the gate works. */
    assert(libssh2_version(0x7FFFFFFF) == NULL);

    printf("libssh2 test OK: %s\n", v);
    return 0;
}
