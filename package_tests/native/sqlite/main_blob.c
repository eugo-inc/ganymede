/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for sqlite: blob; calls sqlite3_open(), sqlite3_exec(), sqlite3_prepare_v2(), sqlite3_bind_blob().
 */

#include <sqlite3.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);

    assert(sqlite3_exec(db, "CREATE TABLE b (data BLOB)", NULL, NULL, NULL) == SQLITE_OK);

    /* Payload contains an embedded NUL and full-byte-range values — anything that treats
     * BLOBs as C strings will truncate at byte 3. */
    const unsigned char payload[] = {
        0xDE, 0xAD, 0xBE, 0x00, 0xEF, 0x01, 0x7F, 0x80, 0xFF, 0x42
    };
    const int payload_len = (int)sizeof(payload);

    sqlite3_stmt *ins = NULL;
    assert(sqlite3_prepare_v2(db, "INSERT INTO b (data) VALUES (?)", -1, &ins, NULL) == SQLITE_OK);
    assert(sqlite3_bind_blob(ins, 1, payload, payload_len, SQLITE_TRANSIENT) == SQLITE_OK);
    assert(sqlite3_step(ins) == SQLITE_DONE);
    sqlite3_finalize(ins);

    sqlite3_stmt *sel = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT data FROM b", -1, &sel, NULL) == SQLITE_OK);
    assert(sqlite3_step(sel) == SQLITE_ROW);

    assert(sqlite3_column_type(sel, 0) == SQLITE_BLOB);
    int got_len = sqlite3_column_bytes(sel, 0);
    assert(got_len == payload_len);

    const void *got = sqlite3_column_blob(sel, 0);
    assert(got != NULL);
    assert(memcmp(got, payload, (size_t)payload_len) == 0);

    sqlite3_finalize(sel);
    sqlite3_close(db);
    printf("sqlite blob test OK (%d bytes round-tripped)\n", payload_len);
    return 0;
}
