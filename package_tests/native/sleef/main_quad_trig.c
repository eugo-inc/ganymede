/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sleef: quad trig; calls Sleef_cast_from_doubleq1(), Sleef_sinq1_u10(), Sleef_cast_to_doubleq1(), Sleef_logq1_u10().
 */

#include <sleefquad.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    /* sin(0) == 0 in quad precision. */
    Sleef_quad zero = Sleef_cast_from_doubleq1(0.0);
    Sleef_quad s = Sleef_sinq1_u10(zero);
    double sd = Sleef_cast_to_doubleq1(s);
    assert(fabs(sd) < 1e-15);

    Sleef_quad one = Sleef_cast_from_doubleq1(1.0);
    Sleef_quad back = Sleef_logq1_u10(Sleef_expq1_u10(one));
    double back_d = Sleef_cast_to_doubleq1(back);
    assert(fabs(back_d - 1.0) < 1e-12);

    /* pow(2, 10) == 1024 in quad precision. */
    Sleef_quad two = Sleef_cast_from_doubleq1(2.0);
    Sleef_quad ten = Sleef_cast_from_doubleq1(10.0);
    Sleef_quad p = Sleef_powq1_u10(two, ten);
    double pd = Sleef_cast_to_doubleq1(p);
    assert(fabs(pd - 1024.0) < 1e-9);

    printf("sleef quad trig OK\n");
    return 0;
}
