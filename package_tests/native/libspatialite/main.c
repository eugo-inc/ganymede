/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libspatialite: core API; calls spatialite_version(), spatialite_initialize(), sqlite3_open(), spatialite_alloc_connection().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sqlite3.h>
#include <spatialite.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *v = spatialite_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    spatialite_initialize();

    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);
    assert(db != NULL);

    void *cache = spatialite_alloc_connection();
    assert(cache != NULL);
    spatialite_init_ex(db, cache, 0);

    sqlite3_stmt *stmt = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT AsText(MakePoint(1.0, 2.0, 4326))",
                              -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    const unsigned char *wkt = sqlite3_column_text(stmt, 0);
    assert(wkt != NULL);
    assert(strstr((const char *)wkt, "POINT") != NULL);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    spatialite_cleanup_ex(cache);
    spatialite_shutdown();

    printf("libspatialite test OK: %s\n", v);
    return 0;
}
