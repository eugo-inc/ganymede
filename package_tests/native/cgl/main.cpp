/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cgl: core API; compiles against <CglGomory.hpp>, <CoinPackedMatrix.hpp>, <OsiClpSolverInterface.hpp>.
 */

#include <CglGomory.hpp>
#include <CoinPackedMatrix.hpp>
#include <OsiClpSolverInterface.hpp>
#include <OsiCuts.hpp>
#include <cstdio>

int main() {
  const CoinBigIndex starts[] = {0, 2, 4};
  const int rows[] = {0, 1, 0, 1};
  const double values[] = {6.0, 1.0, 4.0, 2.0};
  const double column_lower[] = {0.0, 0.0};
  const double column_upper[] = {10.0, 10.0};
  const double objective[] = {-5.0, -4.0};
  const double row_lower[] = {-1.0e30, -1.0e30};
  const double row_upper[] = {24.0, 6.0};
  CoinPackedMatrix matrix(true, 2, 2, 4, values, rows, starts, nullptr);
  OsiClpSolverInterface solver;
  solver.messageHandler()->setLogLevel(0);
  solver.loadProblem(matrix, column_lower, column_upper, objective, row_lower, row_upper);
  solver.setInteger(0);
  solver.setInteger(1);
  solver.initialSolve();
  const double* point = solver.getColSolution();
  CglGomory gomory;
  OsiCuts cuts;
  gomory.generateCuts(solver, cuts);
  if (cuts.sizeRowCuts() < 1) {
    std::printf("FAIL no Gomory cut at (%g, %g)\n", point[0], point[1]);
    return 1;
  }
  for (int i = 0; i < cuts.sizeRowCuts(); ++i) {
    if (cuts.rowCut(i).violated(point) <= 1e-9) {
      std::printf("FAIL cut %d does not cut off (%g, %g)\n", i, point[0], point[1]);
      return 1;
    }
  }
  std::printf("OK Cgl Gomory: %d cut(s) at (%g, %g)\n", cuts.sizeRowCuts(), point[0], point[1]);
  return 0;
}
