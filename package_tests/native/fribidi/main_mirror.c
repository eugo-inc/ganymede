/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fribidi: mirror; calls fribidi_get_mirror_char().
 */

#include <fribidi.h>
#include <assert.h>
#include <stdio.h>

/* Exercise the mirroring API — fribidi_get_mirror_char — which maps
 * a character to its visual mirror for RTL rendering (e.g. '(' -> ')').
 * Distinct from char-type classification and from log2vis. */
int main(void) {
    FriBidiChar mirrored = 0;

    /* '(' (U+0028) must mirror to ')' (U+0029). */
    fribidi_boolean has_mirror = fribidi_get_mirror_char((FriBidiChar)'(', &mirrored);
    assert(has_mirror);
    assert(mirrored == (FriBidiChar)')');

    /* And vice versa. */
    mirrored = 0;
    has_mirror = fribidi_get_mirror_char((FriBidiChar)')', &mirrored);
    assert(has_mirror);
    assert(mirrored == (FriBidiChar)'(');

    /* Square brackets mirror too. */
    mirrored = 0;
    has_mirror = fribidi_get_mirror_char((FriBidiChar)'[', &mirrored);
    assert(has_mirror);
    assert(mirrored == (FriBidiChar)']');

    /* A letter has no mirror — output must be untouched-equal to input
     * AND the function must report no-mirror. */
    mirrored = 0;
    has_mirror = fribidi_get_mirror_char((FriBidiChar)'A', &mirrored);
    assert(!has_mirror);
    assert(mirrored == (FriBidiChar)'A');

    printf("fribidi mirror test OK\n");
    return 0;
}
