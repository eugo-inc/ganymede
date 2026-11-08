/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for utf8proc: casefold; calls utf8proc_tolower(), utf8proc_toupper(), utf8proc_map().
 */

#include <utf8proc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Per-codepoint API: 'A' (U+0041) -> 'a' (U+0061). */
    assert(utf8proc_tolower(0x0041) == 0x0061);
    /* 'Z' -> 'z'. */
    assert(utf8proc_tolower(0x005A) == 0x007A);
    /* Latin capital letter 'É' (U+00C9) -> 'é' (U+00E9). */
    assert(utf8proc_tolower(0x00C9) == 0x00E9);
    /* Inverse: utf8proc_toupper. */
    assert(utf8proc_toupper(0x00E9) == 0x00C9);

    /* Casefold via utf8proc_map: "HELLO" -> "hello". */
    {
        const utf8proc_uint8_t in[] = "HELLO";
        utf8proc_uint8_t *out = NULL;
        utf8proc_ssize_t n = utf8proc_map(
            in, 5, &out,
            (utf8proc_option_t)(UTF8PROC_NULLTERM | UTF8PROC_CASEFOLD | UTF8PROC_STABLE)
        );
        assert(n >= 0);
        assert(out != NULL);
        assert(strcmp((const char *)out, "hello") == 0);
        free(out);
    }

    /* Casefold: German 'ß' (U+00DF, UTF-8: 0xC3 0x9F) folds to "ss" — the
     * load-bearing test that casefold is true Unicode case folding, not
     * a naive per-codepoint tolower. */
    {
        const utf8proc_uint8_t sharp_s[] = {0xC3, 0x9F, 0x00};
        utf8proc_uint8_t *out = NULL;
        utf8proc_ssize_t n = utf8proc_map(
            sharp_s, 2, &out,
            (utf8proc_option_t)(UTF8PROC_CASEFOLD | UTF8PROC_STABLE)
        );
        assert(n >= 0);
        assert(out != NULL);
        assert(strcmp((const char *)out, "ss") == 0);
        free(out);
    }

    printf("utf8proc casefold test OK\n");
    return 0;
}
