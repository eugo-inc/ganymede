/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for z3_solver: real; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_mk_real_sort().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    Z3_config cfg = Z3_mk_config();
    Z3_context ctx = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    Z3_sort real_sort = Z3_mk_real_sort(ctx);
    Z3_symbol x_sym = Z3_mk_string_symbol(ctx, "x");
    Z3_ast x = Z3_mk_const(ctx, x_sym, real_sort);
    Z3_ast two = Z3_mk_real(ctx, 2, 1);
    Z3_ast one = Z3_mk_real(ctx, 1, 1);
    Z3_ast args[2] = {two, x};
    Z3_ast prod = Z3_mk_mul(ctx, 2, args);
    Z3_ast eq = Z3_mk_eq(ctx, prod, one);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    Z3_solver_assert(ctx, solver, eq);
    Z3_lbool result = Z3_solver_check(ctx, solver);
    assert(result == Z3_L_TRUE);

    Z3_model model = Z3_solver_get_model(ctx, solver);
    Z3_model_inc_ref(ctx, model);
    assert(model != NULL);

    Z3_ast x_val = NULL;
    bool ok = Z3_model_eval(ctx, model, x, true, &x_val);
    assert(ok == true);
    assert(x_val != NULL);

    Z3_string num_str = Z3_get_numeral_string(ctx, x_val);
    assert(num_str != NULL);
    /* Z3 returns the rational as "1/2" for Real sort. */
    assert(strcmp(num_str, "1/2") == 0);

    /* Print before the teardown: Z3_get_numeral_string returns a buffer the context owns, and
     * Z3_del_context frees it, so reading num_str afterwards would be a use-after-free. */
    printf("z3_solver real test OK: x = %s\n", num_str);

    Z3_model_dec_ref(ctx, model);
    Z3_solver_dec_ref(ctx, solver);
    Z3_del_context(ctx);
    return 0;
}
