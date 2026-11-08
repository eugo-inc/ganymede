/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for icu: collation; calls u_init(), ucol_open(), u_strFromUTF8(), ucol_strcoll().
 */

#include <unicode/utypes.h>
#include <unicode/ucol.h>
#include <unicode/ustring.h>
#include <unicode/uclean.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    UErrorCode status = U_ZERO_ERROR;

    u_init(&status);
    assert(U_SUCCESS(status));

    /* Open root-locale collator. Loads collation data from libicudata. */
    UCollator *coll = ucol_open("", &status);
    assert(U_SUCCESS(status));
    assert(coll != NULL);

    /* "apple" vs "banana": tertiary (default) strength, apple < banana. */
    UChar apple[8], banana[8], apple_cap[8];
    int32_t apple_len = 0, banana_len = 0, apple_cap_len = 0;
    status = U_ZERO_ERROR;
    u_strFromUTF8(apple, 8, &apple_len, "apple", -1, &status);
    assert(U_SUCCESS(status));
    u_strFromUTF8(banana, 8, &banana_len, "banana", -1, &status);
    assert(U_SUCCESS(status));
    u_strFromUTF8(apple_cap, 8, &apple_cap_len, "Apple", -1, &status);
    assert(U_SUCCESS(status));

    UCollationResult cmp = ucol_strcoll(coll, apple, apple_len, banana, banana_len);
    assert(cmp == UCOL_LESS);

    /* Drop to secondary strength: case-insensitive equality. */
    ucol_setStrength(coll, UCOL_SECONDARY);
    cmp = ucol_strcoll(coll, apple, apple_len, apple_cap, apple_cap_len);
    assert(cmp == UCOL_EQUAL);

    ucol_close(coll);
    printf("icu collation test OK\n");
    return 0;
}
