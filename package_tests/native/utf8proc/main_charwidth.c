/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for utf8proc: charwidth; calls utf8proc_charwidth().
 */

#include <utf8proc.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* ASCII 'A' (U+0041): printable, width 1. */
    assert(utf8proc_charwidth(0x0041) == 1);

    /* ASCII '0' (U+0030): printable, width 1. */
    assert(utf8proc_charwidth(0x0030) == 1);

    /* NUL (U+0000): non-printable control, width 0 in utf8proc's convention
     * (NOT -1 like classical wcwidth). */
    assert(utf8proc_charwidth(0x0000) == 0);

    /* BEL (U+0007): non-printable control, width 0. */
    assert(utf8proc_charwidth(0x0007) == 0);

    /* CJK Unified Ideograph U+4E2D ("中"): East-Asian Wide, width 2. */
    assert(utf8proc_charwidth(0x4E2D) == 2);

    /* Hiragana U+3042 ("あ"): East-Asian Wide, width 2. */
    assert(utf8proc_charwidth(0x3042) == 2);

    /* Combining acute U+0301: zero-width (combines with its base, no cell). */
    assert(utf8proc_charwidth(0x0301) == 0);

    /* U+00A7 SECTION SIGN is East-Asian Ambiguous; utf8proc resolves it to width 1. */
    assert(utf8proc_charwidth(0x00A7) == 1);

    printf("utf8proc charwidth test OK\n");
    return 0;
}
