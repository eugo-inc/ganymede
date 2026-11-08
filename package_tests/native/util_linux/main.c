/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for util_linux: core API; calls uuid_generate(), uuid_is_null(), uuid_unparse(), uuid_parse().
 */

#include <uuid/uuid.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    uuid_t u1;
    uuid_generate(u1);

    /* A freshly-generated UUID must not be the nil UUID. */
    assert(uuid_is_null(u1) == 0);

    char s[37];
    uuid_unparse(u1, s);
    assert(strlen(s) == 36);

    uuid_t u2;
    assert(uuid_parse(s, u2) == 0);
    assert(uuid_compare(u1, u2) == 0);

    /* Two independently-generated UUIDs should differ. */
    uuid_t u3;
    uuid_generate(u3);
    assert(uuid_compare(u1, u3) != 0);

    printf("util_linux test OK: %s\n", s);
    return 0;
}
