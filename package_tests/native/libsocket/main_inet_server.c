/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsocket: inet server; calls create_inet_server_socket(), destroy_inet_socket().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/libinetsocket.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int fd = create_inet_server_socket("127.0.0.1", "0",
                                       LIBSOCKET_TCP, LIBSOCKET_IPv4, 0);
    assert(fd >= 0);

    assert(destroy_inet_socket(fd) == 0);

    printf("libsocket inet_server test OK\n");
    return 0;
}
