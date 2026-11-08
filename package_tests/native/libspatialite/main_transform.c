/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libspatialite: transform; calls spatialite_initialize(), sqlite3_open(), spatialite_alloc_connection(), spatialite_init_ex().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sqlite3.h>
#include <spatialite.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    spatialite_initialize();

    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);
    void *cache = spatialite_alloc_connection();
    assert(cache != NULL);
    spatialite_init_ex(db, cache, 0);

    sqlite3_stmt *init_stmt = NULL;
    assert(sqlite3_prepare_v2(db, "SELECT InitSpatialMetaData(1)",
                              -1, &init_stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(init_stmt) == SQLITE_ROW);
    sqlite3_finalize(init_stmt);

    /* Transform (0,0) lon/lat to Web Mercator → expect (0, 0). */
    sqlite3_stmt *stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT ST_X(ST_Transform(GeomFromText('POINT(0 0)', 4326), 3857)),"
        "       ST_Y(ST_Transform(GeomFromText('POINT(0 0)', 4326), 3857))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    double x0 = sqlite3_column_double(stmt, 0);
    double y0 = sqlite3_column_double(stmt, 1);
    sqlite3_finalize(stmt);
    assert(fabs(x0) < 1e-3);
    assert(fabs(y0) < 1e-3);

    stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT ST_X(ST_Transform(GeomFromText('POINT(90 0)', 4326), 3857))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    double x90 = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    assert(fabs(x90 - 10018754.171) < 1.0);

    stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT ST_X(ST_Transform(ST_Transform("
        "  GeomFromText('POINT(45 30)', 4326), 3857), 4326))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    double rx = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    assert(fabs(rx - 45.0) < 1e-6);

    sqlite3_close(db);
    spatialite_cleanup_ex(cache);
    spatialite_shutdown();

    printf("libspatialite transform test OK\n");
    return 0;
}
