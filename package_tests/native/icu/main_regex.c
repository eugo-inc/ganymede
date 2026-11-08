/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for icu: regex; calls u_init(), u_strFromUTF8(), uregex_open(), uregex_setText().
 */

#include <unicode/utypes.h>
#include <unicode/uregex.h>
#include <unicode/ustring.h>
#include <unicode/uclean.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    UErrorCode status = U_ZERO_ERROR;

    u_init(&status);
    assert(U_SUCCESS(status));

    /* Pattern: "Hello, (\w+)!" — capture the name. */
    const char *pattern_utf8 = "Hello, (\\w+)!";
    UChar pattern[64];
    int32_t pattern_len = 0;
    u_strFromUTF8(pattern, 64, &pattern_len, pattern_utf8, -1, &status);
    assert(U_SUCCESS(status));

    UParseError parse_err;
    URegularExpression *re = uregex_open(pattern, pattern_len, 0, &parse_err, &status);
    assert(U_SUCCESS(status));
    assert(re != NULL);

    /* Input. */
    const char *input_utf8 = "Hello, World!";
    UChar input[64];
    int32_t input_len = 0;
    status = U_ZERO_ERROR;
    u_strFromUTF8(input, 64, &input_len, input_utf8, -1, &status);
    assert(U_SUCCESS(status));

    uregex_setText(re, input, input_len, &status);
    assert(U_SUCCESS(status));

    UBool matched = uregex_matches(re, 0, &status);
    assert(U_SUCCESS(status));
    assert(matched);

    /* Extract capture group 1. */
    UChar group[32];
    int32_t group_len = uregex_group(re, 1, group, 32, &status);
    assert(U_SUCCESS(status));
    assert(group_len == 5);

    char group_utf8[32];
    int32_t group_utf8_len = 0;
    status = U_ZERO_ERROR;
    u_strToUTF8(group_utf8, 32, &group_utf8_len, group, group_len, &status);
    assert(U_SUCCESS(status));
    group_utf8[group_utf8_len] = '\0';
    assert(strcmp(group_utf8, "World") == 0);

    uregex_close(re);
    printf("icu regex test OK: captured '%s'\n", group_utf8);
    return 0;
}
