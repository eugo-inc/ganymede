/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsocket: unix; calls create_unix_dgram_socket(), destroy_unix_socket().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/libunixsocket.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* NULL bind_path => skip unlink+bind; pure socket(AF_UNIX, SOCK_DGRAM). */
    int fd = create_unix_dgram_socket(NULL, 0);
    assert(fd >= 0);

    assert(destroy_unix_socket(fd) == 0);

    printf("libsocket unix-domain test OK\n");
    return 0;
}
