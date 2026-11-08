/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libffi: varargs; calls ffi_prep_cif_var(), ffi_call().
 */

#include <ffi.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

int main(void) {
    ffi_cif cif;
    ffi_type *arg_types[5] = {
        &ffi_type_pointer,   /* char *buf */
        &ffi_type_uint64,    /* size_t size */
        &ffi_type_pointer,   /* const char *fmt */
        &ffi_type_sint,      /* int (variadic) */
        &ffi_type_double,    /* double (variadic) */
    };
    ffi_status status = ffi_prep_cif_var(
        &cif,
        FFI_DEFAULT_ABI,
        3,                   /* nfixedargs: buf, size, fmt are fixed */
        5,                   /* ntotalargs: + int + double */
        &ffi_type_sint,
        arg_types);
    assert(status == FFI_OK);

    char buf[64];
    memset(buf, 0, sizeof(buf));
    char *bufp = buf;
    size_t bufsz = sizeof(buf);
    const char *fmt = "x=%d y=%.2f";
    int x = 42;
    double y = 3.14;

    void *args[5] = { &bufp, &bufsz, &fmt, &x, &y };
    int written = 0;
    ffi_call(&cif, FFI_FN(snprintf), &written, args);

    /* snprintf returns chars-that-would-be-written-excluding-NUL. */
    assert(written > 0);
    assert(strcmp(buf, "x=42 y=3.14") == 0);
    assert(written == (int)strlen(buf));

    printf("libffi varargs OK: ffi_call(snprintf, ...) -> %d, buf=\"%s\"\n",
           written, buf);
    return 0;
}
