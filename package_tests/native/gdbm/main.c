/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for gdbm: core API; calls gdbm_open(), gdbm_store(), gdbm_fetch(), gdbm_close().
 */

#include <gdbm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_gdbm_test_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    unlink(path); /* gdbm_open will create it fresh */

    GDBM_FILE db = gdbm_open(path, 0, GDBM_NEWDB, 0600, NULL);
    assert(db != NULL);

    datum key = { (char *)"hello", 5 };
    datum val = { (char *)"world", 5 };
    assert(gdbm_store(db, key, val, GDBM_INSERT) == 0);

    datum out = gdbm_fetch(db, key);
    assert(out.dptr != NULL);
    assert(out.dsize == 5);
    assert(memcmp(out.dptr, "world", 5) == 0);
    free(out.dptr);

    gdbm_close(db);
    unlink(path);

    const char *v = gdbm_version;
    assert(v != NULL);
    assert(strstr(v, "GDBM") != NULL);

    printf("gdbm test OK: %s\n", v);
    return 0;
}
