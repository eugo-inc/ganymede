/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freetds: login setters; calls dbinit(), dblogin(), dbloginfree(), dbexit().
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

    LOGINREC *login = dblogin();
    assert(login != NULL);

    /* Each macro -> dbsetlname() under the hood; SUCCEED means the field was stored. */
    assert(DBSETLUSER(login, "demo_user") == SUCCEED);
    assert(DBSETLPWD(login, "s3cret") == SUCCEED);
    assert(DBSETLAPP(login, "demo-smoke-test") == SUCCEED);
    assert(DBSETLHOST(login, "localhost") == SUCCEED);

    /* DBSETLCHARSET / DBSETLNATLANG exercise two more field selectors via the
     * same dbsetlname() machinery — broadens the write-path coverage. */
    assert(DBSETLCHARSET(login, "UTF-8") == SUCCEED);
    assert(DBSETLNATLANG(login, "us_english") == SUCCEED);

    assert(DBSETLVERSION(login, DBVERSION_72) == SUCCEED);

    /* DBSETLPACKET expands to dbsetlpacket() — int-valued setter, distinct path. */
    assert(DBSETLPACKET(login, 4096) == SUCCEED);

    dbloginfree(login);
    dbexit();

    printf("freetds login setters OK: all setter macros returned SUCCEED\n");
    return 0;
}
