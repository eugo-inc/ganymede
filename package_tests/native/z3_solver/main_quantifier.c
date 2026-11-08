/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for z3_solver: quantifier; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_mk_int_sort().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    Z3_config cfg = Z3_mk_config();
    Z3_context ctx = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    Z3_sort int_sort = Z3_mk_int_sort(ctx);
    Z3_symbol x_sym = Z3_mk_string_symbol(ctx, "x");
    Z3_app x_app = Z3_to_app(ctx, Z3_mk_const(ctx, x_sym, int_sort));

    Z3_ast x_ast = Z3_app_to_ast(ctx, x_app);
    Z3_ast one = Z3_mk_int(ctx, 1, int_sort);
    Z3_ast plus_args[2] = {x_ast, one};
    Z3_ast x_plus_1 = Z3_mk_add(ctx, 2, plus_args);
    /* body: (x + 1) <= x  — i.e. the negation of the universal claim. */
    Z3_ast body = Z3_mk_le(ctx, x_plus_1, x_ast);

    Z3_app bound[1] = {x_app};
    Z3_ast existential = Z3_mk_exists_const(
        ctx, 0, 1, bound, 0, NULL, body);
    assert(existential != NULL);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    Z3_solver_assert(ctx, solver, existential);
    Z3_lbool result = Z3_solver_check(ctx, solver);
    /* No integer satisfies (x + 1 <= x), so the existential is UNSAT,
     * which proves the original forall holds. */
    assert(result == Z3_L_FALSE);

    Z3_solver_dec_ref(ctx, solver);
    Z3_del_context(ctx);

    printf("z3_solver quantifier test OK\n");
    return 0;
}
