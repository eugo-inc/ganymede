/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for utf8proc: grapheme; calls utf8proc_grapheme_break(), utf8proc_grapheme_break_stateful().
 */

#include <utf8proc.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* 'A' (U+0041) -> 'B' (U+0042): grapheme break BETWEEN them (true). */
    assert(utf8proc_grapheme_break(0x0041, 0x0042) == 1);

    /* 'a' (U+0061) -> U+0301 COMBINING ACUTE ACCENT: no break (the mark stays with its base). */
    assert(utf8proc_grapheme_break(0x0061, 0x0301) == 0);

    /* CR (U+000D) -> LF (U+000A): NO break — the canonical "CRLF is one
     * grapheme cluster" rule from UAX #29. */
    assert(utf8proc_grapheme_break(0x000D, 0x000A) == 0);

    /* LF -> 'A': break (LF terminates a cluster boundary). */
    assert(utf8proc_grapheme_break(0x000A, 0x0041) == 1);

    /* Stateful variant: walking "ÁB" (A, combining-acute, B).
     * Expect break = 0 between (A, acute), then break = 1 between (acute, B). */
    {
        utf8proc_int32_t state = 0;
        assert(utf8proc_grapheme_break_stateful(0x0041, 0x0301, &state) == 0);
        assert(utf8proc_grapheme_break_stateful(0x0301, 0x0042, &state) == 1);
    }

    /* Regional-indicator pair U+1F1FA + U+1F1F8 forms one flag: no break between them. */
    {
        utf8proc_int32_t state = 0;
        assert(utf8proc_grapheme_break_stateful(0x1F1FA, 0x1F1F8, &state) == 0);
    }

    printf("utf8proc grapheme test OK\n");
    return 0;
}
