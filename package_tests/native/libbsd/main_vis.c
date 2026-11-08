/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libbsd: vis; calls strvis(), strunvis().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <bsd/vis.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Source: 'a','\n','b','\t','c','\0' — contains two non-printable bytes. */
    static const char src[] = {'a', '\n', 'b', '\t', 'c', '\0'};

    /* Encode. dst buffer must be 4x src + 1 per strvis(3).
     * VIS_TAB | VIS_NL force \t and \n to be escaped (by default libbsd's vis()
     * treats them as "safe whitespace" and passes them through unencoded);
     * VIS_CSTYLE renders them as C escapes (\\t, \\n) — all printable. */
    char encoded[64];
    int enc_len = strvis(encoded, src, VIS_OCTAL | VIS_CSTYLE | VIS_TAB | VIS_NL);
    assert(enc_len > 0);
    /* strvis returns the length of the encoded string (excluding NUL). */
    assert((size_t)enc_len == strlen(encoded));
    /* Result must be entirely printable ASCII — that's the whole point of vis(). */
    for (size_t i = 0; i < (size_t)enc_len; ++i) {
        unsigned char c = (unsigned char)encoded[i];
        assert(c >= 0x20 && c < 0x7F);
    }
    /* Encoded form must differ from the source (since src had non-printables). */
    assert(strcmp(encoded, src) != 0);

    char decoded[64];
    int dec_len = strunvis(decoded, encoded);
    assert(dec_len >= 0);
    /* Should match the original src bytes (5 chars + NUL). */
    assert(dec_len == 5);
    assert(memcmp(decoded, src, 5) == 0);

    printf("libbsd vis test OK\n");
    return 0;
}
