/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for geos: wkb; calls initGEOS(), GEOSWKTReader_create(), GEOSWKTReader_read(), GEOSWKBWriter_create().
 */

#include <geos_c.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void notice_handler(const char *fmt, ...) { (void)fmt; }

int main(void) {
    initGEOS(notice_handler, notice_handler);

    GEOSWKTReader *wkt_reader = GEOSWKTReader_create();
    assert(wkt_reader != NULL);

    GEOSGeometry *orig = GEOSWKTReader_read(
        wkt_reader, "POLYGON((0 0, 4 0, 4 3, 0 3, 0 0))");
    assert(orig != NULL);

    GEOSWKBWriter *wkb_writer = GEOSWKBWriter_create();
    assert(wkb_writer != NULL);

    size_t wkb_size = 0;
    unsigned char *wkb_bytes = GEOSWKBWriter_write(wkb_writer, orig, &wkb_size);
    assert(wkb_bytes != NULL);
    assert(wkb_size > 0);
    /* Byte-order byte: 0 (big-endian / XDR) or 1 (little-endian / NDR). */
    assert(wkb_bytes[0] == 0x00 || wkb_bytes[0] == 0x01);

    GEOSWKBReader *wkb_reader = GEOSWKBReader_create();
    assert(wkb_reader != NULL);

    GEOSGeometry *roundtripped = GEOSWKBReader_read(wkb_reader, wkb_bytes, wkb_size);
    assert(roundtripped != NULL);

    /* GEOSEquals returns 1 on equality, 0 on inequality, 2 on error. */
    char eq = GEOSEquals(orig, roundtripped);
    assert(eq == 1);

    GEOSGeom_destroy(roundtripped);
    GEOSWKBReader_destroy(wkb_reader);
    GEOSFree(wkb_bytes);
    GEOSWKBWriter_destroy(wkb_writer);
    GEOSGeom_destroy(orig);
    GEOSWKTReader_destroy(wkt_reader);

    finishGEOS();

    printf("geos WKB roundtrip OK (size=%zu)\n", wkb_size);
    return 0;
}
