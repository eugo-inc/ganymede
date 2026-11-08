/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for graphite2: encform; calls gr_count_unicode_characters().
 */

#include <graphite2/Segment.h>
#include <graphite2/Types.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    const void *err = NULL;

    /* (1) ASCII UTF-8: "hello" — 5 code points, no error. */
    const unsigned char utf8_ascii[] = {'h', 'e', 'l', 'l', 'o'};
    err = (const void *)(size_t)0xDEADBEEFu; /* sentinel — must be cleared to NULL */
    size_t n = gr_count_unicode_characters(
        gr_utf8, utf8_ascii, utf8_ascii + sizeof(utf8_ascii), &err);
    assert(n == 5);
    assert(err == NULL);

    /* (2) UTF-8 with one 2-byte sequence: "café" = 'c','a','f',0xC3,0xA9 — 4 code points. */
    const unsigned char utf8_mixed[] = {'c', 'a', 'f', 0xC3, 0xA9};
    err = (const void *)(size_t)0xDEADBEEFu;
    n = gr_count_unicode_characters(
        gr_utf8, utf8_mixed, utf8_mixed + sizeof(utf8_mixed), &err);
    assert(n == 4);
    assert(err == NULL);

    /* (3) UTF-16: three BMP code units 'A','B','C' — 3 code points. */
    const unsigned short utf16[] = {'A', 'B', 'C'};
    err = (const void *)(size_t)0xDEADBEEFu;
    n = gr_count_unicode_characters(
        gr_utf16, utf16, utf16 + (sizeof(utf16) / sizeof(utf16[0])), &err);
    assert(n == 3);
    assert(err == NULL);

    /* (4) UTF-32: two code points U+0041 'A', U+1F600 (a non-BMP scalar). */
    const unsigned int utf32[] = {0x00000041u, 0x0001F600u};
    err = (const void *)(size_t)0xDEADBEEFu;
    n = gr_count_unicode_characters(
        gr_utf32, utf32, utf32 + (sizeof(utf32) / sizeof(utf32[0])), &err);
    assert(n == 2);
    assert(err == NULL);

    /* (5) Invalid UTF-8: lone continuation byte 0x80 — must report error location.
     * Per the header contract, on a structural fault, *pError is set to the offending
     * location (non-NULL). */
    const unsigned char utf8_bad[] = {'a', 0x80, 'b'};
    err = NULL;
    (void)gr_count_unicode_characters(
        gr_utf8, utf8_bad, utf8_bad + sizeof(utf8_bad), &err);
    assert(err != NULL);

    printf("graphite2 encform test OK\n");
    return 0;
}
