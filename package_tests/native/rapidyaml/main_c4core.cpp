/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidyaml: c4core; calls c4::from_chars(), c4::to_chars().
 */

#include <c4/substr.hpp>
#include <c4/charconv.hpp>
#include <cassert>
#include <cstddef>

int main() {
    /* csubstr is c4's read-only string view — backing storage for ryml scalars. */
    c4::csubstr s = "hello, demo";
    assert(s.size() == 11);
    assert(s.first(5) == "hello");
    assert(s.last(4) == "demo");

    c4::csubstr num_str = "12345";
    int parsed = 0;
    bool ok_parse = c4::from_chars(num_str, &parsed);
    assert(ok_parse);
    assert(parsed == 12345);

    /* And the inverse: write an integer into a writable buffer via c4::to_chars. */
    char buf[16] = {0};
    c4::substr writable(buf, sizeof(buf));
    std::size_t written = c4::to_chars(writable, 6789);
    assert(written > 0);
    assert(written <= sizeof(buf));
    c4::csubstr written_view = writable.first(written);
    assert(written_view == "6789");

    return 0;
}
