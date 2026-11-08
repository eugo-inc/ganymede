/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for z3_solver: core API; calls Z3_get_version(), Z3_mk_config(), Z3_mk_context(), Z3_del_config().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Version query — pure header / linkage check. */
    unsigned major = 0, minor = 0, build = 0, revision = 0;
    Z3_get_version(&major, &minor, &build, &revision);
    assert(major >= 4);

    /* Solve a trivially-unsatisfiable formula: x == 1 AND x == 2. */
    Z3_config cfg = Z3_mk_config();
    assert(cfg != NULL);
    Z3_context ctx = Z3_mk_context(cfg);
    assert(ctx != NULL);
    Z3_del_config(cfg);

    Z3_sort int_sort = Z3_mk_int_sort(ctx);
    Z3_symbol x_sym = Z3_mk_string_symbol(ctx, "x");
    Z3_ast x = Z3_mk_const(ctx, x_sym, int_sort);
    Z3_ast one = Z3_mk_int(ctx, 1, int_sort);
    Z3_ast two = Z3_mk_int(ctx, 2, int_sort);
    Z3_ast eq1 = Z3_mk_eq(ctx, x, one);
    Z3_ast eq2 = Z3_mk_eq(ctx, x, two);
    Z3_ast args[2] = {eq1, eq2};
    Z3_ast conj = Z3_mk_and(ctx, 2, args);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    Z3_solver_assert(ctx, solver, conj);
    Z3_lbool result = Z3_solver_check(ctx, solver);
    assert(result == Z3_L_FALSE);

    Z3_solver_dec_ref(ctx, solver);
    Z3_del_context(ctx);

    printf("z3_solver test OK: %u.%u.%u.%u\n", major, minor, build, revision);
    return 0;
}
