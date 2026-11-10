/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for eigen: core API; calls Eigen::AngleAxisd(), Eigen::Vector3d(), Eigen::MatrixXd::Random().
 */

#include <Eigen/Dense>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef EIGEN_USE_BLAS
#error "EIGEN_USE_BLAS is defined although this program did not ask for it"
#endif

#define DEMO_STR2(x) #x
#define DEMO_STR(x) DEMO_STR2(x)

static int fail(const char* what) {
  std::printf("EIGEN-CHECK: FAIL %s\n", what);
  return 1;
}

int main() {
  // 1. LU solve: A x = b with x = (1, 2, 3).
  Eigen::Matrix3d a;
  a << 4, -2, 1, -2, 4, -2, 1, -2, 4;
  Eigen::Vector3d expected(1, 2, 3);
  Eigen::Vector3d b = a * expected;
  Eigen::Vector3d x = a.partialPivLu().solve(b);
  if ((x - expected).norm() > 1e-12) return fail("partialPivLu solve");

  // 2. Symmetric eigenvalues of diag(1, 2, 3) rotated: eigenvalues stay 1, 2, 3.
  Eigen::Matrix3d q = Eigen::AngleAxisd(0.7, Eigen::Vector3d(1, 1, 0).normalized()).toRotationMatrix();
  Eigen::Matrix3d s = q * Eigen::Vector3d(1, 2, 3).asDiagonal() * q.transpose();
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(s);
  if (eig.info() != Eigen::Success) return fail("SelfAdjointEigenSolver did not converge");
  if ((eig.eigenvalues() - Eigen::Vector3d(1, 2, 3)).norm() > 1e-12) return fail("symmetric eigenvalues");

  // 3. Dynamic-size product against a plain loop.
  const int n = 67, k = 45, m = 53;
  Eigen::MatrixXd p = Eigen::MatrixXd::Random(n, k), r = Eigen::MatrixXd::Random(k, m);
  Eigen::MatrixXd c = p * r;
  double worst = 0.0;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) {
      double sum = 0.0;
      for (int l = 0; l < k; ++l) sum += p(i, l) * r(l, j);
      worst = std::fmax(worst, std::fabs(sum - c(i, j)));
    }
  if (worst > 1e-12) return fail("dynamic-size matrix product");

  // 4. The version header is the release's own.
  if (std::strlen(EIGEN_PRERELEASE_VERSION) != 0) return fail("EIGEN_PRERELEASE_VERSION is not empty (installed as a -dev build)");
  const std::string numbers = std::string(DEMO_STR(EIGEN_MAJOR_VERSION)) + "." + DEMO_STR(EIGEN_MINOR_VERSION) + "." +
                              DEMO_STR(EIGEN_PATCH_VERSION);
  if (numbers != EIGEN_VERSION_STRING) return fail("EIGEN_VERSION_STRING is not MAJOR.MINOR.PATCH");

  std::printf("EIGEN-CHECK: PASS %s\n", EIGEN_VERSION_STRING);
  return 0;
}
