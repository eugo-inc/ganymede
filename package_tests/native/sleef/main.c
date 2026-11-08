/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sleef: core API; calls Sleef_sin_u10().
 */

#include <sleef.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    double r = Sleef_sin_u10(0.0);
    assert(r < 1e-9 && r > -1e-9);
    printf("sleef test OK\n");
    return 0;
}
