/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsocket: cpp exception; compiles against <libsocket/exception.hpp>.
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/exception.hpp>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    libsocket::socket_exception e(__FILE__, 42, "probe", false);

    assert(e.mesg.find(__FILE__) != std::string::npos);
    assert(e.mesg.find("42") != std::string::npos);
    assert(e.mesg.find("probe") != std::string::npos);

    std::printf("libsocket++ exception test OK\n");
    return 0;
}
