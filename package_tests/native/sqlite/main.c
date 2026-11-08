/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sqlite: core API; calls sqlite3_open(), sqlite3_exec(), sqlite3_prepare_v2(), sqlite3_step().
 */

#include <sqlite3.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);
    assert(db != NULL);

    char *errmsg = NULL;
    int rc;
    rc = sqlite3_exec(db, "CREATE TABLE t (n INTEGER)", NULL, NULL, &errmsg);
    assert(rc == SQLITE_OK);
    rc = sqlite3_exec(db, "INSERT INTO t VALUES (1), (2), (3)", NULL, NULL, &errmsg);
    assert(rc == SQLITE_OK);

    sqlite3_stmt *stmt = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    int count = sqlite3_column_int(stmt, 0);
    assert(count == 3);
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    printf("sqlite test OK: %s\n", sqlite3_libversion());
    return 0;
}
