/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libspatialite: geom io; calls spatialite_initialize(), sqlite3_open(), spatialite_alloc_connection(), spatialite_init_ex().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <sqlite3.h>
#include <spatialite.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    spatialite_initialize();

    sqlite3 *db = NULL;
    assert(sqlite3_open(":memory:", &db) == SQLITE_OK);
    void *cache = spatialite_alloc_connection();
    assert(cache != NULL);
    spatialite_init_ex(db, cache, 0);

    sqlite3_stmt *stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT AsText(GeomFromWKB(AsBinary("
        "  GeomFromText('LINESTRING(0 0, 3 4)', 4326)), 4326))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    const unsigned char *wkt = sqlite3_column_text(stmt, 0);
    assert(wkt != NULL);
    assert(strstr((const char *)wkt, "LINESTRING") != NULL);
    assert(strstr((const char *)wkt, "0") != NULL);
    assert(strstr((const char *)wkt, "3") != NULL);
    sqlite3_finalize(stmt);

    /* WKB blob must be non-empty for a real geometry. */
    stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT AsBinary(GeomFromText('POINT(7 11)', 4326))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    int wkb_bytes = sqlite3_column_bytes(stmt, 0);
    const void *wkb = sqlite3_column_blob(stmt, 0);
    assert(wkb != NULL);
    /* WKB POINT(x,y) is byte-order(1) + type(4) + 2*double(16) = 21 bytes. */
    assert(wkb_bytes == 21);
    sqlite3_finalize(stmt);

    /* Geometry-introspection: GeometryType + ST_NPoints. */
    stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT GeometryType(GeomFromText('LINESTRING(0 0, 1 1, 2 2)', 4326)),"
        "       ST_NPoints(GeomFromText('LINESTRING(0 0, 1 1, 2 2)', 4326))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    const unsigned char *gtype = sqlite3_column_text(stmt, 0);
    int npoints = sqlite3_column_int(stmt, 1);
    assert(gtype != NULL);
    assert(strstr((const char *)gtype, "LINESTRING") != NULL);
    assert(npoints == 3);
    sqlite3_finalize(stmt);

    /* GLength on a 3-4-5 right triangle hypotenuse must be ~5.0 (planar). */
    stmt = NULL;
    assert(sqlite3_prepare_v2(db,
        "SELECT GLength(GeomFromText('LINESTRING(0 0, 3 4)'))",
        -1, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_step(stmt) == SQLITE_ROW);
    double len = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    assert(fabs(len - 5.0) < 1e-9);

    sqlite3_close(db);
    spatialite_cleanup_ex(cache);
    spatialite_shutdown();

    printf("libspatialite geom-io test OK\n");
    return 0;
}
