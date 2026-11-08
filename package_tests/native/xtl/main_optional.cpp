/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xtl: optional; calls xtl::missing().
 */

#include <xtl/xoptional.hpp>
#include <cassert>
#include <iostream>

int main() {
    /* Engaged optional — wraps a real double. */
    xtl::xoptional<double> engaged(3.14);
    assert(engaged.has_value());
    assert(engaged.value() == 3.14);

    /* Disengaged optional via the xtl::missing<T>() factory — the documented
       xtl idiom for an empty masked value. */
    auto missing = xtl::missing<double>();
    assert(!missing.has_value());

    /* Arithmetic on engaged optional — value propagates; the engaged flag
       must remain true (this is the masked-array contract). */
    xtl::xoptional<double> a(2.0);
    xtl::xoptional<double> b(5.0);
    auto sum = a + b;
    assert(sum.has_value());
    assert(sum.value() == 7.0);

    /* Arithmetic with a disengaged operand must produce a disengaged result —
       the "masked propagates" rule. */
    auto masked_sum = a + missing;
    assert(!masked_sum.has_value());

    std::cout << "xtl xoptional test OK" << std::endl;
    return 0;
}
