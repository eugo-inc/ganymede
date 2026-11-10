/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for osi: core API; compiles against <OsiCuts.hpp>, <OsiRowCut.hpp>.
 */

#include <OsiCuts.hpp>
#include <OsiRowCut.hpp>
#include <cstdio>

int main() {
  const int indices[] = {0, 1};
  const double elements[] = {1.0, 1.0};
  OsiRowCut cut;
  cut.setRow(2, indices, elements);
  cut.setLb(-1.0e30);
  cut.setUb(4.0);
  OsiCuts cuts;
  cuts.insert(cut);
  if (cuts.sizeRowCuts() != 1 || cuts.rowCut(0).ub() != 4.0 || cuts.rowCut(0).row().getNumElements() != 2) {
    std::printf("FAIL cuts=%d\n", cuts.sizeRowCuts());
    return 1;
  }
  std::printf("OK Osi row cut with %d elements\n", cuts.rowCut(0).row().getNumElements());
  return 0;
}
