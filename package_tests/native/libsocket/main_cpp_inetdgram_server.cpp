/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsocket: cpp inetdgram server; compiles against <libsocket/inetserverdgram.hpp>.
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/inetserverdgram.hpp>
#include <cassert>
#include <cstdio>

int main() {
    /* Loopback + ephemeral port. UDP server side — no accept(), no clients. */
    libsocket::inet_dgram_server sock("127.0.0.1", "0", LIBSOCKET_IPv4);

    /* getfd() (inherited from libsocket::socket) must expose a valid fd. */
    int fd = sock.getfd();
    assert(fd >= 0);

    /* sock's virtual destructor runs at scope exit and closes the fd. */
    std::printf("libsocket++ inet_dgram_server test OK\n");
    return 0;
}
