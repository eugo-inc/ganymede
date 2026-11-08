/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fmt: compile; calls fmt::format(), fmt::format_to().
 */

#include <fmt/compile.h>
#include <fmt/format.h>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <string>

int main() {
    std::string s = fmt::format(FMT_COMPILE("{} = {}"), "answer", 42);
    assert(s == "answer = 42");

    std::string padded = fmt::format(FMT_COMPILE("{:>6}"), 17);
    assert(padded == "    17");

    /* format_to with FMT_COMPILE writes into a memory_buffer through the
     * compile-time-parsed path — what spdlog actually invokes. */
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), FMT_COMPILE("v={:.2f}"), 3.14159);
    std::string out(buf.data(), buf.size());
    assert(out == "v=3.14");

    std::printf("fmt compile test OK\n");
    return 0;
}
