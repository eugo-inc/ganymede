/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xtl: xcomplex; compiles against <xtl/xcomplex.hpp>, <complex>.
 */

#include <xtl/xcomplex.hpp>
#include <complex>
#include <cassert>
#include <iostream>

int main() {
    /* Construct from a std::complex — xtl::xcomplex's implicit interop ctor. */
    xtl::xcomplex<double, double, false> z(std::complex<double>(3.0, 4.0));
    assert(z.real() == 3.0);
    assert(z.imag() == 4.0);

    /* Arithmetic: z + z doubles both real and imaginary components. */
    auto w = z + z;
    assert(w.real() == 6.0);
    assert(w.imag() == 8.0);

    assert(z == z);

    std::cout << "xtl xcomplex test OK" << std::endl;
    return 0;
}
