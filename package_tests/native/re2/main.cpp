/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for re2: core API; calls RE2::FullMatch(), RE2::PartialMatch().
 */

#include <re2/re2.h>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    /* Compile a regex with a capture group; verify ok(). */
    RE2 pattern("(\\w+):(\\d+)");
    assert(pattern.ok());

    /* FullMatch must succeed and bind both captures. */
    std::string host;
    int port = 0;
    assert(RE2::FullMatch("demo:42", pattern, &host, &port));
    assert(host == "demo");
    assert(port == 42);

    /* A string that doesn't match the full pattern must be rejected. */
    assert(!RE2::FullMatch("no-colon-here", pattern, &host, &port));

    /* PartialMatch finds the pattern inside a larger string. */
    assert(RE2::PartialMatch("prefix demo:7 suffix", pattern, &host, &port));
    assert(host == "demo");
    assert(port == 7);

    std::cout << "re2 test OK" << std::endl;
    return 0;
}
