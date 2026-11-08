/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for z3_solver: smtlib2; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_parse_smtlib2_string().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    Z3_config cfg = Z3_mk_config();
    Z3_context ctx = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    const char *prog =
        "(declare-const p Bool)\n"
        "(assert (and p (not p)))\n";

    Z3_ast_vector parsed = Z3_parse_smtlib2_string(
        ctx, prog, 0, NULL, NULL, 0, NULL, NULL);
    assert(parsed != NULL);
    Z3_ast_vector_inc_ref(ctx, parsed);

    unsigned n = Z3_ast_vector_size(ctx, parsed);
    assert(n >= 1);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    for (unsigned i = 0; i < n; i++) {
        Z3_ast a = Z3_ast_vector_get(ctx, parsed, i);
        Z3_solver_assert(ctx, solver, a);
    }

    Z3_lbool result = Z3_solver_check(ctx, solver);
    assert(result == Z3_L_FALSE);

    Z3_solver_dec_ref(ctx, solver);
    Z3_ast_vector_dec_ref(ctx, parsed);
    Z3_del_context(ctx);

    printf("z3_solver smtlib2 test OK\n");
    return 0;
}
