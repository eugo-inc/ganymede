/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libunistring: ctype; calls uc_is_alpha(), uc_is_digit(), uc_is_property(), uc_tolower().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <unictype.h>
#include <unicase.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* Basic alphabetic + digit classification. */
    assert(uc_is_alpha((ucs4_t)'A'));
    assert(uc_is_alpha((ucs4_t)'z'));
    assert(uc_is_alpha((ucs4_t)0x00E9));   /* é */
    assert(!uc_is_alpha((ucs4_t)'7'));
    assert(!uc_is_alpha((ucs4_t)' '));

    assert(uc_is_digit((ucs4_t)'0'));
    assert(uc_is_digit((ucs4_t)'9'));
    assert(!uc_is_digit((ucs4_t)'A'));

    /* Uppercase-letter property via the property-table accessor — exercises
     * the uc_property_t machinery, not just the inline shorthands. */
    assert(uc_is_property((ucs4_t)'A', UC_PROPERTY_UPPERCASE));
    assert(!uc_is_property((ucs4_t)'a', UC_PROPERTY_UPPERCASE));
    assert(uc_is_property((ucs4_t)'a', UC_PROPERTY_LOWERCASE));

    /* Single-codepoint case mapping. */
    assert(uc_tolower((ucs4_t)'A') == (ucs4_t)'a');
    assert(uc_tolower((ucs4_t)0x00C9) == (ucs4_t)0x00E9);  /* É -> é */

    printf("libunistring ctype test OK\n");
    return 0;
}
