/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpq: bytea; calls PQescapeBytea(), PQfreemem(), PQunescapeBytea().
 */

#include <libpq-fe.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Raw byte buffer containing the awkward bytes BYTEA escape must handle:
     * NUL, single-quote, backslash, high-bit, printable ASCII. */
    const unsigned char raw[] = { 0x00, 0x27, 0x5c, 0xff, 'A', 'B', 0x01, 0x7f };
    const size_t raw_len = sizeof(raw);

    /* --- PQescapeBytea: verify it produces a valid, NUL-terminated escaped literal. --- */
    size_t escaped_len = 0;
    unsigned char *escaped = PQescapeBytea(raw, raw_len, &escaped_len);
    assert(escaped != NULL);
    /* escaped_len is documented to INCLUDE the trailing NUL, hence > raw_len for non-empty input
     * (every byte expands to >=1 char and the terminator is added). */
    assert(escaped_len > raw_len);
    assert(strlen((const char *)escaped) == escaped_len - 1);

    const char *esc = (const char *)escaped;
    assert(strstr(esc, "''") != NULL); /* the 0x27 was doubled */

    static const char expected[] =
        "\\\\000"      /* 0x00 -> two wire backslashes + three octal digits */
        "''"          /* 0x27 -> doubled single quote */
        "\\\\\\\\"      /* 0x5c -> FOUR wire backslashes (doubled, then doubled again) */
        "\\\\377"      /* 0xff */
        "AB"          /* printable ASCII, verbatim */
        "\\\\001"      /* 0x01 */
        "\\\\177";     /* 0x7f */
    assert(escaped_len == sizeof(expected));           /* 29: 28 bytes + the NUL */
    assert(memcmp(esc, expected, sizeof(expected)) == 0);

    assert(strstr(esc, "\\\\\\\\") != NULL);

    /* And no backslash run may have ODD length: every escape emits them in pairs, so an odd run means
     * one backslash reached the literal unescaped. This is the property that actually rules out
     * injection, rather than the presence of any particular substring. */
    for (const char *p = esc; *p;) {
        if (*p == '\'') {
            assert(*(p + 1) == '\''); /* never a lone quote */
            p += 2;
        } else if (*p == '\\') {
            size_t run = 0;
            while (p[run] == '\\') {
                ++run;
            }
            assert(run % 2 == 0); /* a lone/odd backslash run is the injection shape */
            p += run;
        } else {
            ++p;
        }
    }
    PQfreemem(escaped);

    /* --- PQunescapeBytea: decode the documented PG hex bytea text format ("\x..."). --- */
    /* The same 8 raw bytes, hand-encoded as PG hex output: 00 27 5c ff 41 42 01 7f. */
    const char *hex = "\\x00275cff4142017f";
    size_t unesc_len = 0;
    unsigned char *unesc = PQunescapeBytea((const unsigned char *)hex, &unesc_len);
    assert(unesc != NULL);
    assert(unesc_len == raw_len);
    assert(memcmp(unesc, raw, raw_len) == 0);
    PQfreemem(unesc);

    printf("libpq bytea escape+unescape OK: raw=%zu escaped=%zu\n", raw_len, escaped_len);
    return 0;
}
