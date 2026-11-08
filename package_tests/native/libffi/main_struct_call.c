/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libffi: struct call; calls ffi_prep_cif(), ffi_call().
 */

#include <ffi.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    int    i;
    double d;
    float  f;
} mixed_t;

static mixed_t pack(int i, double d, float f) {
    mixed_t out;
    out.i = i * 2;
    out.d = d + 1.5;
    out.f = f - 0.25f;
    return out;
}

int main(void) {
    /* Describe the struct type to libffi. */
    ffi_type mixed_type;
    ffi_type *mixed_elements[4] = {
        &ffi_type_sint,
        &ffi_type_double,
        &ffi_type_float,
        NULL,
    };
    mixed_type.size = 0;
    mixed_type.alignment = 0;
    mixed_type.type = FFI_TYPE_STRUCT;
    mixed_type.elements = mixed_elements;

    ffi_cif cif;
    ffi_type *arg_types[3] = {
        &ffi_type_sint,
        &ffi_type_double,
        &ffi_type_float,
    };
    ffi_status status = ffi_prep_cif(
        &cif, FFI_DEFAULT_ABI, 3, &mixed_type, arg_types);
    assert(status == FFI_OK);

    int    a = 21;
    double b = 2.5;
    float  c = 1.25f;
    void *args[3] = { &a, &b, &c };

    mixed_t result;
    memset(&result, 0, sizeof(result));
    ffi_call(&cif, FFI_FN(pack), &result, args);

    assert(result.i == 42);
    assert(result.d == 4.0);
    assert(result.f == 1.0f);

    printf("libffi struct_call OK: pack(21, 2.5, 1.25) = {%d, %f, %f}\n",
           result.i, result.d, (double)result.f);
    return 0;
}
