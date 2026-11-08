/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp3: check header; calls nghttp3_check_header_name(), nghttp3_check_header_value().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int check_name(const char *s) {
    return nghttp3_check_header_name((const uint8_t *)s, strlen(s));
}

static int check_value(const char *s) {
    return nghttp3_check_header_value((const uint8_t *)s, strlen(s));
}

int main(void) {
    /* -------- header NAMES -------- */

    /* Valid lower-case token. */
    assert(check_name("content-type") == 1);
    assert(check_name("x-demo-probe") == 1);

    /* Valid pseudo-headers. */
    assert(check_name(":path") == 1);
    assert(check_name(":method") == 1);
    assert(check_name(":status") == 1);

    /* Empty is invalid. */
    assert(nghttp3_check_header_name((const uint8_t *)"", 0) == 0);

    /* Bare ':' (pseudo-header with no actual name) is invalid. */
    assert(check_name(":") == 0);

    assert(check_name("Content-Type") == 0);
    assert(check_name("content-type") == 1);
    assert(check_name("X-Foo") == 0);
    assert(check_name("x-foo") == 1);

    /* Forbidden token chars (space, control, separators). */
    assert(check_name("bad name") == 0);
    assert(check_name("bad\tname") == 0);

    /* -------- header VALUES -------- */

    /* Plain ASCII values are accepted. */
    assert(check_value("text/plain") == 1);
    assert(check_value("application/json; charset=utf-8") == 1);

    /* Empty value is valid (some headers carry no value). */
    assert(nghttp3_check_header_value((const uint8_t *)"", 0) == 1);

    /* Leading or trailing whitespace is invalid per the validator. */
    assert(check_value(" leading-space") == 0);
    assert(check_value("trailing-space ") == 0);
    assert(check_value("\tleading-tab") == 0);

    /* Embedded NUL / control bytes are invalid. Use an explicit length
     * because strlen would stop at the NUL. */
    {
        const uint8_t v_nul[] = {'a', 0x00, 'b'};
        assert(nghttp3_check_header_value(v_nul, sizeof v_nul) == 0);

        const uint8_t v_cr[] = {'a', 0x0d, 'b'};
        assert(nghttp3_check_header_value(v_cr, sizeof v_cr) == 0);

        const uint8_t v_lf[] = {'a', 0x0a, 'b'};
        assert(nghttp3_check_header_value(v_lf, sizeof v_lf) == 0);
    }

    printf("nghttp3 check_header test OK\n");
    return 0;
}
