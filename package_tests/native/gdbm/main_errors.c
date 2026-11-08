/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for gdbm: errors; calls gdbm_open(), gdbm_store(), gdbm_strerror(), gdbm_fetch().
 */

#include <gdbm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_gdbm_err_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    GDBM_FILE db = gdbm_open(path, 0, GDBM_NEWDB, 0600, NULL);
    assert(db != NULL);

    datum k = { (char *)"k", 1 };
    datum v1 = { (char *)"first", 5 };
    datum v2 = { (char *)"second", 6 };

    assert(gdbm_store(db, k, v1, GDBM_INSERT) == 0);

    /* Duplicate INSERT must fail with rc == 1 per the gdbm contract. */
    int rc = gdbm_store(db, k, v2, GDBM_INSERT);
    assert(rc == 1);
    assert(gdbm_errno == GDBM_CANNOT_REPLACE);

    const char *msg = gdbm_strerror(gdbm_errno);
    assert(msg != NULL);
    assert(msg[0] != '\0');

    /* Value must still be the original. */
    datum got = gdbm_fetch(db, k);
    assert(got.dptr != NULL);
    assert(got.dsize == 5);
    assert(memcmp(got.dptr, "first", 5) == 0);
    free(got.dptr);

    /* REPLACE must succeed and overwrite. */
    assert(gdbm_store(db, k, v2, GDBM_REPLACE) == 0);
    datum got2 = gdbm_fetch(db, k);
    assert(got2.dptr != NULL);
    assert(got2.dsize == 6);
    assert(memcmp(got2.dptr, "second", 6) == 0);
    free(got2.dptr);

    gdbm_close(db);
    unlink(path);
    printf("gdbm errors test OK: %s\n", msg);
    return 0;
}
