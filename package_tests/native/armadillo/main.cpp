/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for armadillo: core API; calls arma::solve(), arma::norm(), fit(), arma::det().
 */

#include <armadillo>

#include <cmath>
#include <cstdio>

int main() {
    // A x = b with x = (1, -2, 3) by construction.
    const arma::mat a = {{4.0, -2.0, 1.0}, {-2.0, 4.0, -2.0}, {1.0, -2.0, 4.0}};
    const arma::vec want = {1.0, -2.0, 3.0};
    const arma::vec b = a * want;
    const arma::vec x = arma::solve(a, b);
    const arma::vec t = {0.0, 1.0, 2.0, 3.0, 4.0};
    arma::mat design(5, 2);
    design.col(0).ones();
    design.col(1) = t;
    const arma::vec y = 2.0 + 0.5 * t;
    const arma::vec fit = arma::solve(design, y);
    const double solve_err = arma::norm(x - want, "inf");
    const double fit_err = std::fabs(fit(0) - 2.0) + std::fabs(fit(1) - 0.5);
    const double det_err = std::fabs(arma::det(a) - 36.0);
    std::printf("ARMADILLO-CHECK: solve err %.3e, least-squares err %.3e, det err %.3e (version %s)\n", solve_err, fit_err,
                det_err, arma::arma_version::as_string().c_str());
    if (solve_err > 1e-12 || fit_err > 1e-12 || det_err > 1e-10) {
        std::printf("ARMADILLO-CHECK: FAIL\n");
        return 1;
    }
    std::printf("ARMADILLO-CHECK: PASS\n");
    return 0;
}
