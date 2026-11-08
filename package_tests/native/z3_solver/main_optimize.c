/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for z3_solver: optimize; calls Z3_mk_config(), Z3_mk_context(), Z3_del_config(), Z3_mk_int_sort().
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
    Z3_ast zero = Z3_mk_int(ctx, 0, int_sort);
    Z3_ast ten = Z3_mk_int(ctx, 10, int_sort);
    Z3_ast lo = Z3_mk_ge(ctx, x, zero);
    Z3_ast hi = Z3_mk_le(ctx, x, ten);

    Z3_optimize opt = Z3_mk_optimize(ctx);
    Z3_optimize_inc_ref(ctx, opt);
    Z3_optimize_assert(ctx, opt, lo);
    Z3_optimize_assert(ctx, opt, hi);

    (void)Z3_optimize_maximize(ctx, opt, x);

    Z3_lbool result = Z3_optimize_check(ctx, opt, 0, NULL);
    assert(result == Z3_L_TRUE);

    Z3_model model = Z3_optimize_get_model(ctx, opt);
    Z3_model_inc_ref(ctx, model);
    assert(model != NULL);

    Z3_ast x_val = NULL;
    bool ok = Z3_model_eval(ctx, model, x, true, &x_val);
    assert(ok == true);
    assert(x_val != NULL);

    Z3_string num_str = Z3_get_numeral_string(ctx, x_val);
    assert(num_str != NULL);
    assert(strcmp(num_str, "10") == 0);

    /* Print before the teardown: Z3_get_numeral_string returns a buffer the context owns, and
     * Z3_del_context frees it, so reading num_str afterwards would be a use-after-free. */
    printf("z3_solver optimize test OK: max x = %s\n", num_str);

    Z3_model_dec_ref(ctx, model);
    Z3_optimize_dec_ref(ctx, opt);
    Z3_del_context(ctx);
    return 0;
}
