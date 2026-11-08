/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xsf: gamma; calls xsf::gamma(), xsf::gammaln().
 *
 * xsf is header-only and has no install step of its own, so this spec looks for
 * xsf's include/xsf/ tree under the prefix. To run it, point --prefix at an xsf
 * source checkout (the folder that holds include/xsf/).
 */

#include <xsf/gamma.h>

#include <cmath>
#include <cstdio>

namespace {

bool close_to(double got, double want) { return std::fabs(got - want) <= 1e-13 * std::fabs(want); }

} // namespace

int main() {
    const double gamma_5 = xsf::gamma(5.0);
    if (!close_to(gamma_5, 24.0)) {
        std::fprintf(stderr, "FAIL: xsf::gamma(5.0) = %.17g, expected 24\n", gamma_5);
        return 2;
    }

    const double gamma_half = xsf::gamma(0.5);
    const double sqrt_pi = std::sqrt(std::acos(-1.0));
    if (!close_to(gamma_half, sqrt_pi)) {
        std::fprintf(stderr, "FAIL: xsf::gamma(0.5) = %.17g, expected sqrt(pi) = %.17g\n", gamma_half, sqrt_pi);
        return 3;
    }

    const double ln_gamma_10 = xsf::gammaln(10.0);
    const double ln_9_factorial = std::log(362880.0);
    if (!close_to(ln_gamma_10, ln_9_factorial)) {
        std::fprintf(stderr, "FAIL: xsf::gammaln(10.0) = %.17g, expected ln(362880) = %.17g\n", ln_gamma_10,
                     ln_9_factorial);
        return 4;
    }

    std::printf("xsf OK: gamma(5)=%.17g gamma(0.5)=%.17g gammaln(10)=%.17g\n", gamma_5, gamma_half, ln_gamma_10);
    return 0;
}
