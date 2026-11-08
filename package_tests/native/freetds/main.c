/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freetds: core API; calls dbinit(), dblogin(), dbloginfree(), dbversion().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <sybfront.h>
#include <sybdb.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(dbinit() == SUCCEED);

    LOGINREC *login = dblogin();
    assert(login != NULL);
    dbloginfree(login);

    const char *v = dbversion();
    assert(v != NULL);

    dbexit();

    printf("freetds test OK: %s\n", v);
    return 0;
}
