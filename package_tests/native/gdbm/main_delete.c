/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gdbm: delete; calls gdbm_open(), gdbm_store(), gdbm_exists(), gdbm_fetch().
 */

#include <gdbm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_gdbm_del_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    GDBM_FILE db = gdbm_open(path, 0, GDBM_NEWDB, 0600, NULL);
    assert(db != NULL);

    datum k = { (char *)"to-remove", 9 };
    datum v = { (char *)"payload", 7 };
    assert(gdbm_store(db, k, v, GDBM_INSERT) == 0);

    /* Present before delete. */
    assert(gdbm_exists(db, k) != 0);
    datum got = gdbm_fetch(db, k);
    assert(got.dptr != NULL);
    assert(got.dsize == 7);
    assert(memcmp(got.dptr, "payload", 7) == 0);
    free(got.dptr);

    /* Delete returns 0 on success. */
    assert(gdbm_delete(db, k) == 0);

    /* Absent after delete. */
    assert(gdbm_exists(db, k) == 0);
    datum missing = gdbm_fetch(db, k);
    assert(missing.dptr == NULL);

    /* Deleting a missing key fails (non-zero). */
    assert(gdbm_delete(db, k) != 0);

    gdbm_close(db);
    unlink(path);
    printf("gdbm delete test OK\n");
    return 0;
}
