/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: str split; calls absl::StrSplit(), absl::SkipEmpty(), absl::StrJoin(), absl::SimpleAtoi().
 */

#include <absl/strings/numbers.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

int main() {
    /* (1) Basic split on ','. */
    const std::vector<std::string> parts =
        absl::StrSplit("alpha,beta,gamma,delta", ',');
    assert(parts.size() == 4);
    assert(parts[0] == "alpha");
    assert(parts[1] == "beta");
    assert(parts[2] == "gamma");
    assert(parts[3] == "delta");

    /* (2) SkipEmpty drops empty fragments from "::" / leading ':' / trailing ':'. */
    const std::vector<std::string> nonempty =
        absl::StrSplit(":foo::bar:", ':', absl::SkipEmpty());
    assert(nonempty.size() == 2);
    assert(nonempty[0] == "foo");
    assert(nonempty[1] == "bar");

    /* (3) StrJoin the split tokens back together. */
    const std::string joined = absl::StrJoin(parts, "|");
    assert(joined == "alpha|beta|gamma|delta");

    /* StrJoin a heterogeneous vector<int>. */
    const std::vector<int> nums = {1, 2, 3, 4, 5};
    const std::string nums_joined = absl::StrJoin(nums, "-");
    assert(nums_joined == "1-2-3-4-5");

    /* (4) SimpleAtoi: positive case. */
    int parsed = 0;
    assert(absl::SimpleAtoi("12345", &parsed));
    assert(parsed == 12345);

    /* SimpleAtoi: negative number. */
    int neg = 0;
    assert(absl::SimpleAtoi("-42", &neg));
    assert(neg == -42);

    /* SimpleAtoi: non-numeric input must return false. */
    int garbage = 0;
    assert(!absl::SimpleAtoi("not_a_number", &garbage));

    std::printf("absl str_split test OK: joined=%s parsed=%d\n", joined.c_str(), parsed);
    return 0;
}
