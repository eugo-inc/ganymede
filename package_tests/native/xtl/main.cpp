/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xtl: core API; compiles against <xtl/xbasic_fixed_string.hpp>, <xtl/xtl_config.hpp>.
 */

#include <xtl/xbasic_fixed_string.hpp>
#include <xtl/xtl_config.hpp>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    /* Construct an xfixed_string<32> from a C string — exercises the
       silent_error policy + buffer storage instantiation. */
    xtl::xfixed_string<32> s("demo");
    assert(s.size() == 4);
    assert(s == "demo");

    s += "-xtl";
    assert(s.size() == 8);
    const std::string round = std::string(s.c_str());
    assert(round == "demo-xtl");

    assert(s[0] == 'd');
    assert(s[7] == 'l');

    std::cout << "xtl test OK: "
              << XTL_VERSION_MAJOR << "."
              << XTL_VERSION_MINOR << "."
              << XTL_VERSION_PATCH << std::endl;
    return 0;
}
