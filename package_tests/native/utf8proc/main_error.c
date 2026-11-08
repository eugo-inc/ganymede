/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for utf8proc: error; calls utf8proc_errmsg(), utf8proc_iterate(), utf8proc_codepoint_valid().
 */

#include <utf8proc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* utf8proc_errmsg should return a non-NULL, non-empty string for each
     * documented error code. */
    const utf8proc_ssize_t err_codes[] = {
        UTF8PROC_ERROR_NOMEM,
        UTF8PROC_ERROR_OVERFLOW,
        UTF8PROC_ERROR_INVALIDUTF8,
        UTF8PROC_ERROR_NOTASSIGNED,
        UTF8PROC_ERROR_INVALIDOPTS,
    };
    for (size_t i = 0; i < sizeof(err_codes) / sizeof(err_codes[0]); i++) {
        const char *msg = utf8proc_errmsg(err_codes[i]);
        assert(msg != NULL);
        assert(strlen(msg) > 0);
    }

    /* Invalid UTF-8: 0xFF is never a valid lead byte under any UTF-8 encoding.
     * utf8proc_iterate must return a negative error code and set codepoint = -1. */
    {
        const utf8proc_uint8_t bad[] = {0xFF, 0x00};
        utf8proc_int32_t cp = 0;
        utf8proc_ssize_t consumed = utf8proc_iterate(bad, 1, &cp);
        assert(consumed < 0);
        assert(consumed == UTF8PROC_ERROR_INVALIDUTF8);
        assert(cp == -1);
    }

    /* Invalid UTF-8: truncated sequence. 0xC3 needs a continuation byte but only one byte is given. */
    {
        const utf8proc_uint8_t truncated[] = {0xC3, 0x00};
        utf8proc_int32_t cp = 0;
        utf8proc_ssize_t consumed = utf8proc_iterate(truncated, 1, &cp);
        assert(consumed < 0);
        assert(cp == -1);
    }

    /* 'A' (U+0041) — valid. */
    assert(utf8proc_codepoint_valid(0x0041) == 1);
    /* U+10FFFF — the maximum legal Unicode scalar value. */
    assert(utf8proc_codepoint_valid(0x10FFFF) == 1);
    /* U+110000 — one past the max, invalid. */
    assert(utf8proc_codepoint_valid(0x110000) == 0);
    /* U+D800 — high surrogate, invalid as a standalone scalar. */
    assert(utf8proc_codepoint_valid(0xD800) == 0);
    /* U+DFFF — low surrogate, invalid as a standalone scalar. */
    assert(utf8proc_codepoint_valid(0xDFFF) == 0);
    /* Negative — invalid. */
    assert(utf8proc_codepoint_valid(-1) == 0);

    printf("utf8proc error test OK\n");
    return 0;
}
