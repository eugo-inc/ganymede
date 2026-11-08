/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for z3_solver: model; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_mk_int_sort().
 */

#include <z3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    Z3_config cfg = Z3_mk_config();
    Z3_context ctx = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    Z3_sort int_sort = Z3_mk_int_sort(ctx);
    Z3_symbol x_sym = Z3_mk_string_symbol(ctx, "x");
    Z3_ast x = Z3_mk_const(ctx, x_sym, int_sort);
    Z3_ast lo = Z3_mk_int(ctx, 41, int_sort);
    Z3_ast hi = Z3_mk_int(ctx, 43, int_sort);
    Z3_ast gt = Z3_mk_gt(ctx, x, lo);
    Z3_ast lt = Z3_mk_lt(ctx, x, hi);

    Z3_solver solver = Z3_mk_solver(ctx);
    Z3_solver_inc_ref(ctx, solver);
    Z3_solver_assert(ctx, solver, gt);
    Z3_solver_assert(ctx, solver, lt);
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
    assert(strcmp(num_str, "42") == 0);

    /* Print before the teardown: Z3_get_numeral_string returns a buffer the context owns, and
     * Z3_del_context frees it, so reading num_str afterwards would be a use-after-free. */
    printf("z3_solver model test OK: x = %s\n", num_str);

    Z3_model_dec_ref(ctx, model);
    Z3_solver_dec_ref(ctx, solver);
    Z3_del_context(ctx);
    return 0;
}
