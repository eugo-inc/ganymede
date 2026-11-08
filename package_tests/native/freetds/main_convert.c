/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freetds: convert; calls dbinit(), dbconvert(), dbexit().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <sybfront.h>
#include <sybdb.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(dbinit() == SUCCEED);

    /* SYBINT4 -> SYBCHAR */
    DBINT src_int = 31415926;
    char dest_str[32];
    memset(dest_str, 0, sizeof(dest_str));
    DBINT written = dbconvert(NULL,
                              SYBINT4, (const BYTE *)&src_int, sizeof(src_int),
                              SYBCHAR, (BYTE *)dest_str, (DBINT)sizeof(dest_str) - 1);
    assert(written > 0);
    assert(strstr(dest_str, "31415926") != NULL);

    const char *src_str = "31415926";
    DBINT dest_int = 0;
    DBINT written2 = dbconvert(NULL,
                               SYBCHAR, (const BYTE *)src_str, (DBINT)strlen(src_str),
                               SYBINT4, (BYTE *)&dest_int, sizeof(dest_int));
    assert(written2 == (DBINT)sizeof(DBINT));
    assert(dest_int == src_int);

    dbexit();
    printf("freetds dbconvert OK: '%s' <-> %d\n", dest_str, (int)dest_int);
    return 0;
}
