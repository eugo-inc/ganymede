/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpsl: idn; calls psl_builtin(), psl_str_to_utf8lower(), psl_free_string(), psl_is_public_suffix2().
 */

#include <libpsl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    const psl_ctx_t *psl = psl_builtin();
    assert(psl != NULL);

    /* --- Aspect 1: UTF-8 lowercasing through the libidn2 path. ----------------------- */
    char *lower = NULL;
    psl_error_t rc = psl_str_to_utf8lower("EXAMPLE.COM", "utf-8", NULL, &lower);
    assert(rc == PSL_SUCCESS);
    assert(lower != NULL);
    assert(strcmp(lower, "example.com") == 0);
    psl_free_string(lower);
    lower = NULL;

    /* Mixed case + already lower mix — must produce all-lower output. */
    rc = psl_str_to_utf8lower("WwW.ExAmPlE.OrG", "utf-8", NULL, &lower);
    assert(rc == PSL_SUCCESS);
    assert(lower != NULL);
    assert(strcmp(lower, "www.example.org") == 0);
    psl_free_string(lower);

    /* --- Aspect 2: ICANN vs PRIVATE suffix type filtering. --------------------------- */
    assert(psl_is_public_suffix2(psl, "com", PSL_TYPE_ICANN | PSL_TYPE_NO_STAR_RULE) == 1);
    assert(psl_is_public_suffix2(psl, "com", PSL_TYPE_PRIVATE | PSL_TYPE_NO_STAR_RULE) == 0);
    assert(psl_is_public_suffix2(psl, "com", PSL_TYPE_ANY) == 1);
    /* Without NO_STAR_RULE, both ICANN-only and PRIVATE-only return 1 for a TLD (star match). */
    assert(psl_is_public_suffix2(psl, "com", PSL_TYPE_PRIVATE) == 1);

    /* "github.io" is in the PRIVATE section of the PSL — multi-label, so the star rule does
     * not apply and the type discriminator works directly. */
    assert(psl_is_public_suffix2(psl, "github.io", PSL_TYPE_PRIVATE) == 1);
    assert(psl_is_public_suffix2(psl, "github.io", PSL_TYPE_ICANN) == 0);
    assert(psl_is_public_suffix2(psl, "github.io", PSL_TYPE_ANY) == 1);

    /* "example.com" is not a public suffix under any type. */
    assert(psl_is_public_suffix2(psl, "example.com", PSL_TYPE_ANY) == 0);

    printf("libpsl idn/type test OK\n");
    return 0;
}
