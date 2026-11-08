/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzip: error; calls zip_open(), zip_error_init_with_code(), zip_error_code_zip(), zip_error_strerror().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zip.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* zip_open on a guaranteed-missing path must fail with ZIP_ER_NOENT. */
    int err = 0;
    zip_t *za = zip_open("/tmp/demo_libzip_definitely_not_here.zip", ZIP_RDONLY, &err);
    assert(za == NULL);
    assert(err == ZIP_ER_OPEN || err == ZIP_ER_NOENT);

    zip_error_t ze;
    zip_error_init_with_code(&ze, ZIP_ER_NOENT);
    assert(zip_error_code_zip(&ze) == ZIP_ER_NOENT);

    const char *msg = zip_error_strerror(&ze);
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    zip_error_fini(&ze);

    /* A fresh, default-initialized zip_error_t carries the OK code. */
    zip_error_t ok_err;
    zip_error_init(&ok_err);
    assert(zip_error_code_zip(&ok_err) == ZIP_ER_OK);
    zip_error_fini(&ok_err);

    printf("libzip error-API test OK\n");
    return 0;
}
