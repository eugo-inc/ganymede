/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fmt: core API; calls fmt::format().
 */

#include <fmt/core.h>
#include <fmt/format.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* Basic positional + integer formatting — exercises the core format engine. */
    std::string s = fmt::format("{} = {}", "answer", 42);
    assert(s == "answer = 42");

    /* Width / fill / alignment — exercises the parser + writer paths. */
    std::string padded = fmt::format("{:>5}", 7);
    assert(padded == "    7");

    /* FMT_VERSION is a compile-time int (e.g. 120100 for 12.1.0). */
    static_assert(FMT_VERSION > 0, "fmt version macro must be defined");

    std::printf("fmt test OK: FMT_VERSION=%d\n", FMT_VERSION);
    return 0;
}
