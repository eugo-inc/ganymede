/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fmt: printf; compiles against <fmt/printf.h>.
 */

#include <fmt/printf.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* Basic %d / %s — exercises the printf-style arg dispatch. */
    std::string s = fmt::sprintf("%s = %d", "answer", 42);
    assert(s == "answer = 42");

    /* %.Nf float precision — exercises the printf-style float formatter. */
    std::string pi = fmt::sprintf("pi=%.3f", 3.14159);
    assert(pi == "pi=3.142");

    /* %x hex — exercises the printf-style integer formatter w/ alt-base. */
    std::string hex = fmt::sprintf("0x%04x", 255);
    assert(hex == "0x00ff");

    /* Positional args (%1$s %2$d) — exercises fmt's printf positional path,
     * which is a POSIX extension many printf impls support and which fmt
     * implements distinctly from the Python-style "{0}/{1}" positional path. */
    std::string pos = fmt::sprintf("%2$d-%1$s", "x", 7);
    assert(pos == "7-x");

    std::printf("fmt printf test OK\n");
    return 0;
}
