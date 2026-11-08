/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgpg_error: core API; calls gpg_strerror(), gpg_strsource(), gpg_err_make(), gpg_err_code().
 */

#include <gpg-error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Error-string lookup: must produce non-empty, non-NULL strings. */
    const char *ok_str = gpg_strerror(GPG_ERR_NO_ERROR);
    assert(ok_str != NULL);
    assert(strlen(ok_str) > 0);

    const char *src_str = gpg_strsource(GPG_ERR_SOURCE_GPG);
    assert(src_str != NULL);
    assert(strlen(src_str) > 0);

    gpg_error_t err = gpg_err_make(GPG_ERR_SOURCE_GPG, GPG_ERR_INV_VALUE);
    assert(gpg_err_code(err) == GPG_ERR_INV_VALUE);
    assert(gpg_err_source(err) == GPG_ERR_SOURCE_GPG);

    printf("libgpg_error test OK: %s\n", ok_str);
    return 0;
}
