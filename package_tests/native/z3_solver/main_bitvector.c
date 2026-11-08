/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for z3_solver: bitvector; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_mk_bv_sort().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    Z3_config cfg = Z3_mk_config();
    Z3_context ctx = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    Z3_sort bv32 = Z3_mk_bv_sort(ctx, 32);
    Z3_symbol x_sym = Z3_mk_string_symbol(ctx, "x");
    Z3_ast x = Z3_mk_const(ctx, x_sym, bv32);
    Z3_ast one = Z3_mk_unsigned_int(ctx, 1, bv32);
    Z3_ast zero = Z3_mk_unsigned_int(ctx, 0, bv32);
    Z3_ast sum = Z3_mk_bvadd(ctx, x, one);
    Z3_ast eq = Z3_mk_eq(ctx, sum, zero);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    Z3_solver_assert(ctx, solver, eq);
    Z3_lbool result = Z3_solver_check(ctx, solver);
    assert(result == Z3_L_TRUE);

    Z3_solver_dec_ref(ctx, solver);
    Z3_del_context(ctx);

    printf("z3_solver bitvector test OK\n");
    return 0;
}
