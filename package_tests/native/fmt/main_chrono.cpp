/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fmt: chrono; calls fmt::format().
 */

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <string>

int main() {
    /* Duration with unit suffix — fmt knows about std::chrono::seconds. */
    std::string dur = fmt::format("{}", std::chrono::seconds(42));
    assert(dur == "42s");

    /* Milliseconds get the "ms" suffix. */
    std::string ms = fmt::format("{}", std::chrono::milliseconds(7));
    assert(ms == "7ms");

    std::tm tm{};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 3 - 1;
    tm.tm_mday = 15;
    tm.tm_hour = 13;
    tm.tm_min = 45;
    tm.tm_sec = 30;
    std::string ts = fmt::format("{:%Y-%m-%d %H:%M:%S}", tm);
    assert(ts == "2024-03-15 13:45:30");

    std::printf("fmt chrono test OK\n");
    return 0;
}
