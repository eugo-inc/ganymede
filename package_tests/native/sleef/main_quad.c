/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sleef: quad; calls Sleef_cast_from_doubleq1(), Sleef_addq1_u05(), Sleef_cast_to_doubleq1(), Sleef_sqrtq1_u05().
 */

#include <sleefquad.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    Sleef_quad a = Sleef_cast_from_doubleq1(2.0);
    Sleef_quad b = Sleef_cast_from_doubleq1(3.0);

    Sleef_quad sum = Sleef_addq1_u05(a, b);
    double sum_d = Sleef_cast_to_doubleq1(sum);
    assert(fabs(sum_d - 5.0) < 1e-12);

    Sleef_quad two = Sleef_cast_from_doubleq1(2.0);
    Sleef_quad root = Sleef_sqrtq1_u05(two);
    double root_d = Sleef_cast_to_doubleq1(root);
    assert(fabs(root_d - sqrt(2.0)) < 1e-12);

    printf("sleef quad OK\n");
    return 0;
}
