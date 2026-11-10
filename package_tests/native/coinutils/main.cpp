/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for coinutils: core API; compiles against <CoinPackedMatrix.hpp>.
 */

#include <CoinPackedMatrix.hpp>
#include <cstdio>

int main() {
  // [[1 0 2], [0 3 4]] in column-major sparse form.
  const CoinBigIndex starts[] = {0, 1, 2, 4};
  const int rows[] = {0, 1, 0, 1};
  const double values[] = {1.0, 3.0, 2.0, 4.0};
  const int lengths[] = {1, 1, 2};
  CoinPackedMatrix matrix(true, 2, 3, 4, values, rows, starts, lengths);
  const double x[] = {1.0, 2.0, 3.0};
  double y[2] = {0.0, 0.0};
  matrix.times(x, y);
  const double u[] = {1.0, 1.0};
  double v[3] = {0.0, 0.0, 0.0};
  matrix.transposeTimes(u, v);
  if (y[0] != 7.0 || y[1] != 18.0 || v[0] != 1.0 || v[1] != 3.0 || v[2] != 6.0) {
    std::printf("FAIL times %g %g, transposeTimes %g %g %g\n", y[0], y[1], v[0], v[1], v[2]);
    return 1;
  }
  std::printf("OK CoinUtils: A x = (%g, %g), A^T u = (%g, %g, %g)\n", y[0], y[1], v[0], v[1], v[2]);
  return 0;
}
