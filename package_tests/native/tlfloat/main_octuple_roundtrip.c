/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tlfloat: octuple roundtrip; calls tlfloat_cast_o_d_(), tlfloat_cast_d_o().
 */

#include <assert.h>

#include <tlfloat/tlfloat.h>

int main(void) {
  tlfloat_octuple_ wide = tlfloat_cast_o_d_(2.5);
  double back = tlfloat_cast_d_o(wide);
  assert(back == 2.5);
  return 0;
}
