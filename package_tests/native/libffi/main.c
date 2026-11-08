/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libffi: core API; calls ffi_prep_cif(), ffi_call().
 */

#include <ffi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

int main(void) {
    ffi_cif cif;
    ffi_type *arg_types[1] = { &ffi_type_pointer };
    ffi_status status = ffi_prep_cif(
        &cif, FFI_DEFAULT_ABI, 1, &ffi_type_uint64, arg_types);
    assert(status == FFI_OK);

    /* Dispatch strlen("libffi") through libffi's call machinery. */
    const char *input = "libffi";
    void *args[1] = { &input };
    size_t result = 0;
    ffi_call(&cif, FFI_FN(strlen), &result, args);

    assert(result == strlen(input));
    assert(result == 6);

    printf("libffi test OK: ffi_call(strlen, \"libffi\") = %zu\n", result);
    return 0;
}
