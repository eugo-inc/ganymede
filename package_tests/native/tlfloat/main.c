/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tlfloat: core API; calls tlfloat_version().
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <tlfloat/tlfloat.h>

int main(void) {
  uint64_t version = tlfloat_version();
  printf("tlfloat_version() = %llu\n", (unsigned long long)version);
  /* tlfloat_version() encodes MAJOR*1000000 + MINOR*1000 + PATCH; any release reports a positive value. */
  assert(version > 0);
  return 0;
}
