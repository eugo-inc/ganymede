/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for clp: core API; compiles against <ClpSimplex.hpp>, <CoinPackedMatrix.hpp>, <OsiClpSolverInterface.hpp>.
 */

#include <ClpSimplex.hpp>
#include <CoinPackedMatrix.hpp>
#include <OsiClpSolverInterface.hpp>
#include <cmath>
#include <cstdio>

int main() {
  const CoinBigIndex starts[] = {0, 2, 4};
  const int rows[] = {0, 1, 0, 1};
  const double values[] = {1.0, 1.0, 1.0, 3.0};
  const double column_lower[] = {0.0, 0.0};
  const double column_upper[] = {1.0e30, 1.0e30};
  const double objective[] = {-1.0, -2.0};
  const double row_lower[] = {-1.0e30, -1.0e30};
  const double row_upper[] = {4.0, 6.0};
  CoinPackedMatrix matrix(true, 2, 2, 4, values, rows, starts, nullptr);
  ClpSimplex model;
  model.setLogLevel(0);
  model.loadProblem(matrix, column_lower, column_upper, objective, row_lower, row_upper);
  model.primal();
  const double* solution = model.primalColumnSolution();
  if (!model.isProvenOptimal() || std::fabs(model.objectiveValue() + 5.0) > 1e-9 || std::fabs(solution[0] - 3.0) > 1e-9 ||
      std::fabs(solution[1] - 1.0) > 1e-9) {
    std::printf("FAIL clp objective %g\n", model.objectiveValue());
    return 1;
  }
  OsiClpSolverInterface osi;
  osi.messageHandler()->setLogLevel(0);
  osi.loadProblem(matrix, column_lower, column_upper, objective, row_lower, row_upper);
  osi.initialSolve();
  if (!osi.isProvenOptimal() || std::fabs(osi.getObjValue() + 5.0) > 1e-9) {
    std::printf("FAIL osi-clp objective %g\n", osi.getObjValue());
    return 1;
  }
  std::printf("OK Clp and OsiClp optimum %g\n", model.objectiveValue());
  return 0;
}
