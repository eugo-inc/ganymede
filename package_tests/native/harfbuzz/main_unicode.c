/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for harfbuzz: unicode; calls hb_unicode_funcs_get_default(), hb_unicode_general_category(), hb_unicode_script(), hb_unicode_mirroring().
 */

#include <hb.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    hb_unicode_funcs_t *ufuncs = hb_unicode_funcs_get_default();
    assert(ufuncs != NULL);

    /* General category: 'A' is uppercase letter, '5' is decimal digit, ' ' is space. */
    assert(hb_unicode_general_category(ufuncs, 'A')
           == HB_UNICODE_GENERAL_CATEGORY_UPPERCASE_LETTER);
    assert(hb_unicode_general_category(ufuncs, '5')
           == HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER);
    assert(hb_unicode_general_category(ufuncs, ' ')
           == HB_UNICODE_GENERAL_CATEGORY_SPACE_SEPARATOR);

    /* Script lookup across three different writing systems. */
    assert(hb_unicode_script(ufuncs, 'a') == HB_SCRIPT_LATIN);
    /* U+03B1 GREEK SMALL LETTER ALPHA. */
    assert(hb_unicode_script(ufuncs, 0x03B1) == HB_SCRIPT_GREEK);
    /* U+3042 HIRAGANA LETTER A. */
    assert(hb_unicode_script(ufuncs, 0x3042) == HB_SCRIPT_HIRAGANA);

    /* Mirroring: '(' mirrors to ')' and vice versa. */
    assert(hb_unicode_mirroring(ufuncs, '(') == ')');
    assert(hb_unicode_mirroring(ufuncs, ')') == '(');
    /* Non-mirrored char mirrors to itself. */
    assert(hb_unicode_mirroring(ufuncs, 'A') == 'A');

    assert(hb_unicode_combining_class(ufuncs, 'A')
           == HB_UNICODE_COMBINING_CLASS_NOT_REORDERED);

    printf("harfbuzz unicode-funcs test OK\n");
    return 0;
}
