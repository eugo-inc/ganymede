/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidyaml: float reads; calls c4::yml::from_chars_float(), c4::to_csubstr(), ryml::parse_in_arena().
 */

#include <ryml.hpp>
#include <ryml_std.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

namespace {

int failures = 0;
constexpr double kInf = std::numeric_limits<double>::infinity();

template <typename T>
bool same(T got, T want) {
    if (std::isnan(want)) return std::isnan(got);
    return got == want && std::signbit(got) == std::signbit(want);
}

template <typename T>
void expect(const char *text, bool accepted, T want = T(0)) {
    T got = T(-3.5);
    const bool ok = c4::yml::from_chars_float(c4::to_csubstr(text), &got);
    if (ok != accepted || (ok && !same(got, want))) {
        std::printf("FAIL: from_chars_float<%s>(\"%s\"): accepted=%d value=%.17g (want accepted=%d value=%.17g)\n",
                    sizeof(T) == 4 ? "float" : "double", text, ok, static_cast<double>(got), accepted,
                    static_cast<double>(want));
        ++failures;
    }
}

void expect_document(const char *scalar, bool accepted, double want = 0.0) {
    const std::string doc = std::string("v: ") + scalar + "\n";
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(doc));
    double got = -3.5;
    const bool ok = c4::yml::from_chars_float(tree["v"].val(), &got);
    if (ok != accepted || (ok && !same(got, want))) {
        std::printf("FAIL: 'v: %s' read: accepted=%d value=%.17g (want accepted=%d value=%.17g)\n", scalar, ok, got,
                    accepted, want);
        ++failures;
    }
}

}  // namespace

int main() {
    expect<double>("1.5", true, 1.5);
    expect<double>("+1.5", true, 1.5);
    expect<double>("-1.5E-3", true, -0.0015);
    expect<double>("0.30000000000000004", true, 0.30000000000000004);
    expect<double>("9007199254740993", true, 9007199254740992.0);
    expect<double>("4.9e-324", true, 4.9406564584124654e-324);
    expect<double>(".5", true, 0.5);
    expect<double>("5.", true, 5.0);
    expect<double>("1e", true, 1.0);
    expect<double>("1.5x", true, 1.5);
    expect<double>("inf", true, kInf);
    expect<double>("-inf", true, -kInf);
    expect<double>("infinity", true, kInf);
    expect<double>("nan", true, std::numeric_limits<double>::quiet_NaN());
    expect<double>("1e400", false);
    expect<double>("-1e400", false);
    expect<double>("1e-400", false);
    expect<double>("2.4e-324", false);
    expect<double>("1.7976931348623159e308", false);
    expect<float>("0.30000000000000004", true, 0.30000001192092896f);
    expect<float>("1.4e-45", true, 1.401298464324817e-45f);
    expect<float>("3.4028236e38", false);
    expect<float>("7.0e-46", false);
    expect<float>("4.9e-324", false);

    expect_document("1.5", true, 1.5);
    expect_document("-1.5E-3", true, -0.0015);
    expect_document("0.30000000000000004", true, 0.30000000000000004);
    expect_document("1e400", false);
    expect_document("1e-400", false);

    if (failures != 0) {
        std::printf("rapidyaml float reads: %d case(s) FAILED\n", failures);
        return 1;
    }
    std::printf("rapidyaml float reads: all cases OK\n");
    return 0;
}
