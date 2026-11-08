/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for utf8proc: category; calls utf8proc_category(), utf8proc_get_property(), utf8proc_category_string().
 */

#include <utf8proc.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* 'A' (U+0041) is Lu (Letter, uppercase). */
    assert(utf8proc_category(0x0041) == UTF8PROC_CATEGORY_LU);

    /* 'a' (U+0061) is Ll (Letter, lowercase). */
    assert(utf8proc_category(0x0061) == UTF8PROC_CATEGORY_LL);

    /* '5' (U+0035) is Nd (Number, decimal digit). */
    assert(utf8proc_category(0x0035) == UTF8PROC_CATEGORY_ND);

    /* ' ' (U+0020) is Zs (Separator, space). */
    assert(utf8proc_category(0x0020) == UTF8PROC_CATEGORY_ZS);

    /* U+0301 COMBINING ACUTE ACCENT is Mn (Mark, nonspacing). */
    assert(utf8proc_category(0x0301) == UTF8PROC_CATEGORY_MN);

    /* The combining acute also has combining class 230 — a load-bearing
     * fact for canonical reordering. Verify via utf8proc_get_property. */
    const utf8proc_property_t *p = utf8proc_get_property(0x0301);
    assert(p != NULL);
    assert(p->combining_class == 230);

    /* 'A' (U+0041) has combining class 0 (starter). */
    p = utf8proc_get_property(0x0041);
    assert(p != NULL);
    assert(p->combining_class == 0);

    /* Category name lookup is non-NULL for a real category. */
    const char *cat_name = utf8proc_category_string(0x0041);
    assert(cat_name != NULL);
    /* "Lu" is the expected short name. */
    assert(cat_name[0] == 'L' && cat_name[1] == 'u' && cat_name[2] == '\0');

    printf("utf8proc category test OK\n");
    return 0;
}
