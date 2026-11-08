/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fmt: ostream; calls fmt::format(), fmt::streamed(), fmt::print().
 */

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <cassert>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <string>

struct Point {
    int x;
    int y;
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "(" << p.x << "," << p.y << ")";
}

int main() {
    Point p{3, 7};
    std::string s = fmt::format("pt={}", fmt::streamed(p));
    assert(s == "pt=(3,7)");

    /* fmt::print can target an arbitrary std::ostream. */
    std::ostringstream oss;
    fmt::print(oss, "id={:04d}", 42);
    assert(oss.str() == "id=0042");

    std::printf("fmt ostream test OK\n");
    return 0;
}
