/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for re2: set; compiles against <re2/re2.h>, <re2/set.h>.
 */

#include <re2/re2.h>
#include <re2/set.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    RE2::Set set(RE2::DefaultOptions, RE2::UNANCHORED);

    /* Add three patterns; Add() returns the index for each. */
    std::string err;
    int idx_word   = set.Add("\\w+", &err);
    int idx_digits = set.Add("\\d+", &err);
    int idx_hex    = set.Add("0x[0-9a-fA-F]+", &err);
    assert(idx_word == 0);
    assert(idx_digits == 1);
    assert(idx_hex == 2);
    assert(set.Size() == 3);

    /* Compile the set DFA. */
    assert(set.Compile());

    /* Input has letters, digits, AND a hex literal → all 3 patterns match. */
    {
        std::vector<int> matches;
        bool ok = set.Match("value 0xCAFE 42", &matches);
        assert(ok);
        std::sort(matches.begin(), matches.end());
        assert(matches.size() == 3);
        assert(matches[0] == 0);
        assert(matches[1] == 1);
        assert(matches[2] == 2);
    }

    /* Letters only → \\w+ matches; \\d+ and 0x... do not. */
    {
        std::vector<int> matches;
        bool ok = set.Match("hello", &matches);
        assert(ok);
        assert(matches.size() == 1);
        assert(matches[0] == 0);
    }

    /* Empty input matches none. */
    {
        std::vector<int> matches;
        bool ok = set.Match("", &matches);
        assert(!ok);
        assert(matches.empty());
    }

    std::cout << "re2 set test OK" << std::endl;
    return 0;
}
