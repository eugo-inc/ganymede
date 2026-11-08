/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for icu: break iterator; calls u_init(), u_strFromUTF8(), ubrk_open(), ubrk_first().
 */

#include <unicode/utypes.h>
#include <unicode/ubrk.h>
#include <unicode/ustring.h>
#include <unicode/uclean.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    UErrorCode status = U_ZERO_ERROR;

    u_init(&status);
    assert(U_SUCCESS(status));

    const char *input = "Hello world foo";
    UChar utf16[32];
    int32_t utf16_len = 0;
    u_strFromUTF8(utf16, 32, &utf16_len, input, -1, &status);
    assert(U_SUCCESS(status));

    /* Open a word-boundary iterator for English. */
    UBreakIterator *it = ubrk_open(UBRK_WORD, "en", utf16, utf16_len, &status);
    assert(U_SUCCESS(status));
    assert(it != NULL);

    int32_t pos = ubrk_first(it);
    assert(pos == 0);

    int32_t boundary_count = 0;
    int32_t last_pos = 0;
    while ((pos = ubrk_next(it)) != UBRK_DONE) {
        assert(pos > last_pos);
        last_pos = pos;
        ++boundary_count;
    }
    /* "Hello world foo" has at least 3 word-segments + their spaces. */
    assert(boundary_count >= 3);
    assert(last_pos == utf16_len);

    ubrk_close(it);
    printf("icu break-iterator test OK (%d boundaries)\n", boundary_count);
    return 0;
}
