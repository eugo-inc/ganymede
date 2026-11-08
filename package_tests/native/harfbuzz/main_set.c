/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for harfbuzz: set; calls hb_set_create(), hb_set_allocation_successful(), hb_set_is_empty(), hb_set_get_population().
 */

#include <hb.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    hb_set_t *s = hb_set_create();
    assert(s != NULL);
    assert(hb_set_allocation_successful(s));
    assert(hb_set_is_empty(s));
    assert(hb_set_get_population(s) == 0);

    /* Add three distinct codepoints; membership + population. */
    hb_set_add(s, 'A');
    hb_set_add(s, 'B');
    hb_set_add(s, 0x3042); /* HIRAGANA LETTER A */
    assert(hb_set_has(s, 'A'));
    assert(hb_set_has(s, 'B'));
    assert(hb_set_has(s, 0x3042));
    assert(!hb_set_has(s, 'C'));
    assert(hb_set_get_population(s) == 3);
    assert(!hb_set_is_empty(s));

    /* Min / max. */
    assert(hb_set_get_min(s) == 'A');
    assert(hb_set_get_max(s) == 0x3042);

    /* Delete one — population drops. */
    hb_set_del(s, 'B');
    assert(!hb_set_has(s, 'B'));
    assert(hb_set_get_population(s) == 2);

    /* Range insertion: add [10, 19] inclusive. */
    hb_set_add_range(s, 10, 19);
    assert(hb_set_get_population(s) == 12);
    assert(hb_set_has(s, 10));
    assert(hb_set_has(s, 15));
    assert(hb_set_has(s, 19));
    assert(!hb_set_has(s, 9));
    assert(!hb_set_has(s, 20));

    /* Union with another set. */
    hb_set_t *t = hb_set_create();
    hb_set_add(t, 100);
    hb_set_add(t, 200);
    hb_set_union(s, t);
    assert(hb_set_has(s, 100));
    assert(hb_set_has(s, 200));
    assert(hb_set_get_population(s) == 14);

    /* Clear empties the set. */
    hb_set_clear(s);
    assert(hb_set_is_empty(s));
    assert(hb_set_get_population(s) == 0);
    assert(!hb_set_has(s, 'A'));

    hb_set_destroy(t);
    hb_set_destroy(s);

    /* Empty-set singleton: non-NULL, empty, idempotent. */
    hb_set_t *empty = hb_set_get_empty();
    assert(empty != NULL);
    assert(hb_set_is_empty(empty));
    assert(hb_set_get_empty() == empty);

    printf("harfbuzz set test OK\n");
    return 0;
}
