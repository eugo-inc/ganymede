/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freexl: version format; calls freexl_version(), isprint().
 */

#include <freexl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *v1 = freexl_version();
    assert(v1 != NULL);
    assert(strlen(v1) > 0);

    /* First char must be a digit (versions start with "2.0.0", "1.x", ...). */
    assert(isdigit((unsigned char) v1[0]));

    /* Must contain at least one dot — a major-only version is suspect. */
    assert(strchr(v1, '.') != NULL);

    /* Idempotency: same pointer (or at least same content) across calls. */
    const char *v2 = freexl_version();
    assert(v2 != NULL);
    assert(strcmp(v1, v2) == 0);

    /* Length sanity — real versions are short, not hundreds of chars. */
    assert(strlen(v1) < 64);

    /* Every character must be printable. */
    for (const char *p = v1; *p != '\0'; ++p) {
        assert(isprint((unsigned char) *p));
    }

    printf("freexl version-format test OK: %s\n", v1);
    return 0;
}
