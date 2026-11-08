/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for re2: replace; calls RE2::Replace(), RE2::GlobalReplace().
 */

#include <re2/re2.h>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    /* Replace: first occurrence only, with a backreference rewrite. */
    {
        std::string s = "yabba dabba doo";
        bool replaced = RE2::Replace(&s, "b+", "d");
        assert(replaced);
        assert(s == "yada dabba doo");
    }

    /* GlobalReplace: every non-overlapping occurrence, returns the count. */
    {
        std::string s = "yabba dabba doo";
        int n = RE2::GlobalReplace(&s, "b+", "d");
        assert(n == 2);
        assert(s == "yada dada doo");
    }

    /* GlobalReplace with capture-group backreferences (\\1, \\2 in rewrite). */
    {
        std::string s = "alice=1, bob=2, carol=3";
        int n = RE2::GlobalReplace(&s, "(\\w+)=(\\d+)", "\\2:\\1");
        assert(n == 3);
        assert(s == "1:alice, 2:bob, 3:carol");
    }

    /* No match → Replace returns false and leaves the string untouched. */
    {
        std::string s = "no digits here";
        bool replaced = RE2::Replace(&s, "\\d+", "X");
        assert(!replaced);
        assert(s == "no digits here");
    }

    /* No match → GlobalReplace returns 0. */
    {
        std::string s = "no digits here";
        int n = RE2::GlobalReplace(&s, "\\d+", "X");
        assert(n == 0);
        assert(s == "no digits here");
    }

    std::cout << "re2 replace test OK" << std::endl;
    return 0;
}
