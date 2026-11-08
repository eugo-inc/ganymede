/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libspatialite: predicates; calls sqlite3_prepare_v2(), sqlite3_step(), sqlite3_column_int(), sqlite3_finalize().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sqlite3.h>
#include <spatialite.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int scalar_int(sqlite3 *db, const char *sql) {
    sqlite3_stmt *stmt = NULL;
    assert(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    int v = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return v;
}

int main(void) {
    spatialite_initialize();

    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);
    void *cache = spatialite_alloc_connection();
    assert(cache != NULL);
    spatialite_init_ex(db, cache, 0);

    /* Two overlapping polygons — must intersect, must NOT be disjoint. */
    int intersects = scalar_int(db,
        "SELECT ST_Intersects("
        "  GeomFromText('POLYGON((0 0, 4 0, 4 4, 0 4, 0 0))'),"
        "  GeomFromText('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))");
    assert(intersects == 1);

    int disjoint = scalar_int(db,
        "SELECT ST_Disjoint("
        "  GeomFromText('POLYGON((0 0, 4 0, 4 4, 0 4, 0 0))'),"
        "  GeomFromText('POLYGON((2 2, 6 2, 6 6, 2 6, 2 2))'))");
    assert(disjoint == 0);

    /* Outer polygon strictly contains inner point. */
    int contains = scalar_int(db,
        "SELECT ST_Contains("
        "  GeomFromText('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))'),"
        "  GeomFromText('POINT(5 5)'))");
    assert(contains == 1);

    /* Two clearly disjoint geometries — must NOT intersect. */
    int separate = scalar_int(db,
        "SELECT ST_Intersects("
        "  GeomFromText('POINT(0 0)'),"
        "  GeomFromText('POINT(100 100)'))");
    assert(separate == 0);

    sqlite3_close(db);
    spatialite_cleanup_ex(cache);
    spatialite_shutdown();

    printf("libspatialite predicates test OK\n");
    return 0;
}
