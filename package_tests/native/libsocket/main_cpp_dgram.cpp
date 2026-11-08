/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsocket: cpp dgram; compiles against <libsocket/inetclientdgram.hpp>.
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/inetclientdgram.hpp>
#include <cassert>
#include <cstdio>

int main() {
    /* IPv4 UDP, no connect. Internally calls create_inet_dgram_socket. */
    libsocket::inet_dgram_client sock(LIBSOCKET_IPv4);

    /* getfd() exposes the underlying kernel fd; must be a real, open fd. */
    int fd = sock.getfd();
    assert(fd >= 0);

    /* sock's destructor will tear it down here. */
    std::printf("libsocket++ inet_dgram_client test OK\n");
    return 0;
}
