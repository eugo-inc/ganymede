/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsocket: core API; calls create_inet_dgram_socket(), destroy_inet_socket().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/libinetsocket.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Create an unbound IPv4 UDP socket. Exercises a real libsocket entry
     * point that internally calls socket(AF_INET, SOCK_DGRAM, 0). */
    int fd = create_inet_dgram_socket(LIBSOCKET_IPv4, 0);
    assert(fd >= 0);

    /* Tear it down via libsocket's own destructor (not bare close()). */
    assert(destroy_inet_socket(fd) == 0);

    printf("libsocket test OK\n");
    return 0;
}
