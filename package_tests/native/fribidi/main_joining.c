/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fribidi: joining; calls fribidi_get_joining_type(), fribidi_get_joining_types().
 */

#include <fribidi.h>
#include <assert.h>
#include <stdio.h>

/* Exercise the Arabic joining-type API — fribidi_get_joining_type +
 * fribidi_get_joining_types — used to drive Arabic cursive shaping.
 * Distinct from Bidi reordering, mirroring, and bidi-type lookup. */
int main(void) {
    /* Arabic letter BEH (U+0628) is a dual-joining letter (type D). */
    FriBidiJoiningType jt_beh = fribidi_get_joining_type((FriBidiChar)0x0628);
    assert(FRIBIDI_IS_JOINING_TYPE_D(jt_beh));

    /* Arabic letter ALEF (U+0627) is right-joining only (type R). */
    FriBidiJoiningType jt_alef = fribidi_get_joining_type((FriBidiChar)0x0627);
    assert(FRIBIDI_IS_JOINING_TYPE_R(jt_alef));

    /* Latin 'A' is a non-joining character (type U). */
    FriBidiJoiningType jt_a = fribidi_get_joining_type((FriBidiChar)'A');
    assert(FRIBIDI_IS_JOINING_TYPE_U(jt_a));

    /* Bulk-lookup API: feed a 3-char buffer and verify entries match
     * the single-char API for each. */
    FriBidiChar str[3] = { 0x0628, 0x0627, (FriBidiChar)'A' };
    FriBidiJoiningType jts[3];
    fribidi_get_joining_types(str, 3, jts);
    assert(jts[0] == jt_beh);
    assert(jts[1] == jt_alef);
    assert(jts[2] == jt_a);

    printf("fribidi joining test OK\n");
    return 0;
}
