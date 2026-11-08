/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sqlite: bind; calls sqlite3_open(), sqlite3_exec(), sqlite3_prepare_v2(), sqlite3_bind_int().
 */

#include <sqlite3.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);

    assert(sqlite3_exec(db,
        "CREATE TABLE kv (k INTEGER PRIMARY KEY, v TEXT)",
        NULL, NULL, NULL) == SQLITE_OK);

    sqlite3_stmt *ins = NULL;
    assert(sqlite3_prepare_v2(db, "INSERT INTO kv (k, v) VALUES (?, ?)", -1, &ins, NULL) == SQLITE_OK);
    assert(sqlite3_bind_int(ins, 1, 42) == SQLITE_OK);
    assert(sqlite3_bind_text(ins, 2, "hello world", -1, SQLITE_TRANSIENT) == SQLITE_OK);
    assert(sqlite3_step(ins) == SQLITE_DONE);
    sqlite3_finalize(ins);

    sqlite3_stmt *sel = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT v FROM kv WHERE k = ?", -1, &sel, NULL) == SQLITE_OK);
    assert(sqlite3_bind_int(sel, 1, 42) == SQLITE_OK);
    assert(sqlite3_step(sel) == SQLITE_ROW);

    const unsigned char *got = sqlite3_column_text(sel, 0);
    assert(got != NULL);
    assert(strcmp((const char *)got, "hello world") == 0);
    assert(sqlite3_column_bytes(sel, 0) == 11);

    /* No second row. */
    assert(sqlite3_step(sel) == SQLITE_DONE);
    sqlite3_finalize(sel);

    sqlite3_close(db);
    printf("sqlite bind/text test OK\n");
    return 0;
}
