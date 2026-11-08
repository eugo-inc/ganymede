/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fmt: ranges; calls fmt::format(), fmt::join().
 */

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

int main() {
    /* fmt::join — sequence + separator → formatted string. */
    std::vector<int> nums = {1, 2, 3, 4};
    std::string csv = fmt::format("{}", fmt::join(nums, ","));
    assert(csv == "1,2,3,4");

    std::string hex = fmt::format("{:#04x}", fmt::join(std::vector<int>{15, 255}, "-"));
    assert(hex == "0x0f-0xff");

    std::vector<std::string> words = {"alpha", "beta"};
    std::string rendered = fmt::format("{}", words);
    /* fmt renders std::vector<std::string> as ["alpha", "beta"]. */
    assert(rendered == "[\"alpha\", \"beta\"]");

    std::printf("fmt ranges test OK\n");
    return 0;
}
