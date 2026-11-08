/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nettle: sexp; calls sexp_iterator_first(), sexp_iterator_enter_list(), sexp_iterator_next(), sexp_iterator_check_type().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/sexp.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Hand-crafted canonical S-expression: (3:key5:value) */
    static const uint8_t input[] = "(3:key5:value)";
    const size_t input_len = sizeof(input) - 1;

    /* --- Initialize iterator on the top-level expression. --- */
    struct sexp_iterator it;
    int ok = sexp_iterator_first(&it, input_len, input);
    assert(ok == 1);
    /* Top-level is a list. */
    assert(it.type == SEXP_LIST);

    /* --- Descend into the list. --- */
    ok = sexp_iterator_enter_list(&it);
    assert(ok == 1);

    /* First element: atom "key". */
    assert(it.type == SEXP_ATOM);
    assert(it.atom_length == 3);
    assert(memcmp(it.atom, "key", 3) == 0);

    /* Advance to the next element. */
    ok = sexp_iterator_next(&it);
    assert(ok == 1);

    /* Second element: atom "value". */
    assert(it.type == SEXP_ATOM);
    assert(it.atom_length == 5);
    assert(memcmp(it.atom, "value", 5) == 0);

    /* Advance once more — must hit SEXP_END (no more elements in list). */
    ok = sexp_iterator_next(&it);
    assert(ok == 1);
    assert(it.type == SEXP_END);

    /* --- Type-check API: a list keyed by "key" should match. --- */
    struct sexp_iterator it2;
    ok = sexp_iterator_first(&it2, input_len, input);
    assert(ok == 1);
    ok = sexp_iterator_check_type(&it2, "key");
    assert(ok == 1);
    /* The iterator now points at the FIRST element after the type tag, i.e.
     * the atom "value". */
    assert(it2.type == SEXP_ATOM);
    assert(it2.atom_length == 5);
    assert(memcmp(it2.atom, "value", 5) == 0);

    /* --- Negative path: a mismatched type tag must be rejected. --- */
    struct sexp_iterator it3;
    ok = sexp_iterator_first(&it3, input_len, input);
    assert(ok == 1);
    int bad = sexp_iterator_check_type(&it3, "nope");
    assert(bad == 0);

    /* --- Negative path: malformed input (truncated atom length) must
     * fail to initialize the iterator. --- */
    static const uint8_t bad_input[] = "(9:abc)";  /* claims 9 bytes, has 3 */
    struct sexp_iterator it4;
    int init_ok = sexp_iterator_first(&it4, sizeof(bad_input) - 1, bad_input);
    if (init_ok) {
        int enter_ok = sexp_iterator_enter_list(&it4);
        assert(enter_ok == 0);
    }

    printf("nettle sexp test OK\n");
    return 0;
}
