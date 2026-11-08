/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libffi: closure; calls ffi_closure_alloc(), ffi_prep_cif(), ffi_prep_closure_loc(), ffi_closure_free().
 */

#include <ffi.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Generic dispatcher: receives marshalled args via *args[], writes return via *ret. */
static void multiplier_handler(ffi_cif *cif, void *ret, void **args, void *user_data) {
    (void)cif;
    int a = *(int *)args[0];
    int b = *(int *)args[1];
    int factor = *(int *)user_data;
    *(ffi_arg *)ret = (ffi_arg)(a * b * factor);
}

/* Function pointer signature the foreign caller sees. */
typedef int (*int_binop_t)(int, int);

int main(void) {
    /* Allocate a closure + its executable trampoline. */
    void *code_ptr = NULL;
    ffi_closure *closure = ffi_closure_alloc(sizeof(ffi_closure), &code_ptr);
    assert(closure != NULL);
    assert(code_ptr != NULL);

    /* Describe int (*)(int, int). */
    ffi_cif cif;
    ffi_type *arg_types[2] = { &ffi_type_sint, &ffi_type_sint };
    ffi_status status = ffi_prep_cif(
        &cif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, arg_types);
    assert(status == FFI_OK);

    /* Bind the dispatcher + user_data to the trampoline at code_ptr. */
    int factor = 7;
    status = ffi_prep_closure_loc(
        closure, &cif, multiplier_handler, &factor, code_ptr);
    assert(status == FFI_OK);

    /* Invoke through the closure as if it were a plain C function. */
    int_binop_t fn = (int_binop_t)code_ptr;
    int result = fn(3, 5);

    assert(result == 105);

    ffi_closure_free(closure);

    printf("libffi closure OK: closure(3, 5) [factor=7] = %d\n", result);
    return 0;
}
