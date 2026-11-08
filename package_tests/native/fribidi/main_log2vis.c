/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fribidi: log2vis; calls fribidi_log2vis().
 */

#include <fribidi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Exercise the high-level logical-to-visual reordering API
 * (fribidi_log2vis) — the core Bidi algorithm entry point.
 * Feeds an RTL string (Hebrew letters) and confirms the reorder
 * actually reverses the visual order. */
int main(void) {
    /* Hebrew alef (U+05D0), bet (U+05D1), gimel (U+05D2) — strong RTL. */
    FriBidiChar logical[3] = { 0x05D0, 0x05D1, 0x05D2 };
    FriBidiChar visual[3] = { 0, 0, 0 };
    FriBidiParType base_dir = FRIBIDI_PAR_ON;
    FriBidiStrIndex positions_L_to_V[3];
    FriBidiStrIndex positions_V_to_L[3];
    FriBidiLevel embedding_levels[3];

    fribidi_boolean ok = fribidi_log2vis(
        logical, 3, &base_dir,
        visual,
        positions_L_to_V, positions_V_to_L,
        embedding_levels);
    assert(ok);

    /* Base paragraph direction must have resolved to RTL because every
     * character is strong-RTL. */
    assert(FRIBIDI_IS_RTL(base_dir));

    /* RTL reordering must reverse the run: visual[0] = logical[2], etc. */
    assert(visual[0] == logical[2]);
    assert(visual[1] == logical[1]);
    assert(visual[2] == logical[0]);

    /* Embedding levels for pure-RTL run must be odd. */
    for (int i = 0; i < 3; i++) {
        assert((embedding_levels[i] & 1) == 1);
    }

    printf("fribidi log2vis test OK\n");
    return 0;
}
