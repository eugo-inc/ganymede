/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for icu: core API; calls u_init(), u_getVersion(), u_versionToString(), u_strFromUTF8().
 */

#include <unicode/utypes.h>
#include <unicode/uclean.h>
#include <unicode/uversion.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    UErrorCode status = U_ZERO_ERROR;

    /* Initialize ICU — must succeed before any other call. */
    u_init(&status);
    assert(U_SUCCESS(status));

    /* Version string must be non-empty. */
    UVersionInfo version;
    u_getVersion(version);
    char version_str[U_MAX_VERSION_STRING_LENGTH];
    u_versionToString(version, version_str);
    assert(strlen(version_str) > 0);

    /* UTF-8 -> UTF-16 conversion. */
    const char *input = "hello";
    UChar utf16[16];
    int32_t utf16_len = 0;
    status = U_ZERO_ERROR;
    u_strFromUTF8(utf16, 16, &utf16_len, input, -1, &status);
    assert(U_SUCCESS(status));
    assert(utf16_len == 5);

    /* Case mapping (exercises libicudata via the case-folding tables). */
    for (int32_t i = 0; i < utf16_len; ++i) {
        utf16[i] = (UChar)u_toupper(utf16[i]);
    }

    char output[16];
    int32_t output_len = 0;
    status = U_ZERO_ERROR;
    u_strToUTF8(output, 16, &output_len, utf16, utf16_len, &status);
    assert(U_SUCCESS(status));
    output[output_len] = '\0';
    assert(strcmp(output, "HELLO") == 0);

    printf("icu test OK: %s\n", version_str);
    return 0;
}
