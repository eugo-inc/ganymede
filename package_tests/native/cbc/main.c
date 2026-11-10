/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cbc: core API; calls Cbc_newModel(), Cbc_setLogLevel(), Cbc_loadProblem(), Cbc_setInteger().
 */

#include <Cbc_C_Interface.h>
#include <math.h>
#include <stdio.h>

int main(void) {
  const CoinBigIndex starts[] = {0, 2, 4};
  const int rows[] = {0, 1, 0, 1};
  const double values[] = {6.0, 1.0, 4.0, 2.0};
  const double column_lower[] = {0.0, 0.0};
  const double column_upper[] = {10.0, 10.0};
  const double objective[] = {-5.0, -4.0};
  const double row_lower[] = {-1.0e30, -1.0e30};
  const double row_upper[] = {24.0, 6.0};
  Cbc_Model *model = Cbc_newModel();
  Cbc_setLogLevel(model, 0);
  Cbc_loadProblem(model, 2, 2, starts, rows, values, column_lower, column_upper, objective, row_lower, row_upper);
  Cbc_setInteger(model, 0);
  Cbc_setInteger(model, 1);
  Cbc_solve(model);
  const double *solution = Cbc_getColSolution(model);
  const int ok = Cbc_isProvenOptimal(model) && fabs(Cbc_getObjValue(model) + 20.0) < 1e-9 && fabs(solution[0] - 4.0) < 1e-9 &&
                 fabs(solution[1]) < 1e-9;
  printf("%s CBC objective %g at (%g, %g)\n", ok ? "OK" : "FAIL", Cbc_getObjValue(model), solution[0], solution[1]);
  Cbc_deleteModel(model);
  return ok ? 0 : 1;
}
