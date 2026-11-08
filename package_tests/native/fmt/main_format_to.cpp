/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fmt: format to; calls fmt::format_to(), fmt::format_to_n(), fmt::formatted_size().
 */

#include <fmt/core.h>
#include <fmt/format.h>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>

int main() {
    /* fmt::format_to writes through an output iterator into a memory_buffer. */
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "pi={:.3f} e={:.3f}", 3.14159, 2.71828);
    std::string s(buf.data(), buf.size());
    assert(s == "pi=3.142 e=2.718");

    /* fmt::format_to_n caps the write at N chars and reports both bytes-written
     * and total bytes the full format would have produced. */
    char raw[8] = {0};
    auto result = fmt::format_to_n(raw, sizeof(raw) - 1, "{:s}", "abcdefghij");
    assert(result.size == 10);                    /* total size the format wanted */
    assert(static_cast<size_t>(result.out - raw) == 7); /* actually written */
    raw[7] = '\0';
    assert(std::strcmp(raw, "abcdefg") == 0);

    /* fmt::formatted_size reports the byte count the format would produce
     * without actually formatting — used for pre-allocation. */
    size_t sz = fmt::formatted_size("{}-{:04d}", "id", 42);
    assert(sz == 7); /* "id-0042" */

    std::printf("fmt format_to test OK\n");
    return 0;
}
