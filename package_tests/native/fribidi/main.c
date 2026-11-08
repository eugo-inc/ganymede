/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fribidi: core API; calls fribidi_get_bidi_type().
 */

#include <fribidi.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Verify fribidi version + bidi-type lookups for strong LTR and RTL characters. */
    const char *v = fribidi_version_info;
    assert(v != NULL);
    FriBidiCharType ct = fribidi_get_bidi_type('A');
    assert(ct == FRIBIDI_TYPE_LTR);
    assert(fribidi_get_bidi_type(0x05D0) == FRIBIDI_TYPE_RTL); /* HEBREW LETTER ALEF */
    printf("fribidi test OK\n");
    return 0;
}
