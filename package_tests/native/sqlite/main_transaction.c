/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sqlite: transaction; calls sqlite3_prepare_v2(), sqlite3_step(), sqlite3_column_int(), sqlite3_finalize().
 */

#include <sqlite3.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int row_count(sqlite3 *db) {
    sqlite3_stmt *s = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &s, NULL) == SQLITE_OK);
    assert(sqlite3_step(s) == SQLITE_ROW);
    int n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    return n;
}

int main(void) {
    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);

    assert(sqlite3_exec(db, "CREATE TABLE t (n INTEGER)", NULL, NULL, NULL) == SQLITE_OK);

    assert(sqlite3_exec(db, "BEGIN", NULL, NULL, NULL) == SQLITE_OK);
    assert(sqlite3_exec(db, "INSERT INTO t VALUES (10), (20)", NULL, NULL, NULL) == SQLITE_OK);
    assert(sqlite3_exec(db, "COMMIT", NULL, NULL, NULL) == SQLITE_OK);
    assert(row_count(db) == 2);

    /* Rollback path: BEGIN → INSERT → ROLLBACK → row count unchanged. */
    assert(sqlite3_exec(db, "BEGIN", NULL, NULL, NULL) == SQLITE_OK);
    assert(sqlite3_exec(db, "INSERT INTO t VALUES (30), (40), (50)", NULL, NULL, NULL) == SQLITE_OK);
    assert(row_count(db) == 5);  /* visible inside the txn */
    assert(sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL) == SQLITE_OK);
    assert(row_count(db) == 2);  /* reverted */

    /* sqlite3_changes reflects the most recent statement, not lifetime. */
    assert(sqlite3_exec(db, "INSERT INTO t VALUES (60)", NULL, NULL, NULL) == SQLITE_OK);
    assert(sqlite3_changes(db) == 1);

    sqlite3_close(db);
    printf("sqlite transaction test OK\n");
    return 0;
}
