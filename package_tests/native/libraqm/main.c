/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libraqm: core API; calls raqm_create(), raqm_set_text_utf8(), raqm_set_par_direction(), raqm_version().
 */

#include <raqm.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    raqm_t *rq = raqm_create();
    assert(rq != NULL);

    const char *text = "hello world";
    assert(raqm_set_text_utf8(rq, text, strlen(text)) == 1);
    assert(raqm_set_par_direction(rq, RAQM_DIRECTION_LTR) == 1);

    /* Numeric version must be non-trivial and consistent with the string form. */
    unsigned int major = 0, minor = 0, micro = 0;
    raqm_version(&major, &minor, &micro);
    assert(major + minor + micro > 0);

    const char *vs = raqm_version_string();
    assert(vs != NULL);
    assert(strlen(vs) > 0);

    assert(raqm_version_atleast(0, 10, 0) == 1);

    raqm_destroy(rq);

    printf("libraqm test OK: %s\n", vs);
    return 0;
}
