/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgpg_error: errno; calls gpg_err_code_from_errno(), gpg_err_code_to_errno(), gpg_err_make_from_errno(), gpg_err_code().
 */

#include <gpg-error.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int main(void) {
    /* Zero is the documented identity. */
    assert(gpg_err_code_from_errno(0) == 0);

    /* A well-known errno must produce a non-zero gpg_err_code. */
    gpg_err_code_t code = gpg_err_code_from_errno(EINVAL);
    assert(code != 0);

    int recovered_errno = gpg_err_code_to_errno(code);
    assert(recovered_errno == EINVAL);

    gpg_error_t err = gpg_err_make_from_errno(GPG_ERR_SOURCE_USER_1, EINVAL);
    assert(gpg_err_code(err) == code);
    assert(gpg_err_source(err) == GPG_ERR_SOURCE_USER_1);

    printf("libgpg_error errno-mapping test OK: code=%u for EINVAL=%d\n",
           (unsigned)code, EINVAL);
    return 0;
}
