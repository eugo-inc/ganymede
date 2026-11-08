/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for utf8proc: encode; calls utf8proc_encode_char(), utf8proc_iterate().
 */

#include <utf8proc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    utf8proc_uint8_t buf[8];

    /* 1-byte: 'A' (U+0041). */
    memset(buf, 0, sizeof(buf));
    utf8proc_ssize_t n = utf8proc_encode_char(0x0041, buf);
    assert(n == 1);
    assert(buf[0] == 0x41);

    /* 2-byte: 'é' (U+00E9 -> 0xC3 0xA9). */
    memset(buf, 0, sizeof(buf));
    n = utf8proc_encode_char(0x00E9, buf);
    assert(n == 2);
    assert(buf[0] == 0xC3 && buf[1] == 0xA9);

    /* 3-byte: '€' (U+20AC -> 0xE2 0x82 0xAC). */
    memset(buf, 0, sizeof(buf));
    n = utf8proc_encode_char(0x20AC, buf);
    assert(n == 3);
    assert(buf[0] == 0xE2 && buf[1] == 0x82 && buf[2] == 0xAC);

    /* 4-byte: U+1F600 GRINNING FACE -> 0xF0 0x9F 0x98 0x80. */
    memset(buf, 0, sizeof(buf));
    n = utf8proc_encode_char(0x1F600, buf);
    assert(n == 4);
    assert(buf[0] == 0xF0 && buf[1] == 0x9F && buf[2] == 0x98 && buf[3] == 0x80);

    /* Round-trip: encode then iterate recovers the codepoint. */
    memset(buf, 0, sizeof(buf));
    n = utf8proc_encode_char(0x1F600, buf);
    utf8proc_int32_t cp = 0;
    utf8proc_ssize_t consumed = utf8proc_iterate(buf, n, &cp);
    assert(consumed == n);
    assert(cp == 0x1F600);

    printf("utf8proc encode test OK\n");
    return 0;
}
