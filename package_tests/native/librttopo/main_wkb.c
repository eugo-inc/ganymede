/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for librttopo: wkb; calls rtgeom_init(), rtpoint_make2d(), rtpoint_as_rtgeom(), rtgeom_to_hexwkb().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <librttopo_geom.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    RTCTX *ctx = rtgeom_init(malloc, realloc, free);
    assert(ctx != NULL);

    /* Construct a 2D point at (3.14, 2.71) in WGS84. */
    RTPOINT *p = rtpoint_make2d(ctx, 4326, 3.14, 2.71);
    assert(p != NULL);
    RTGEOM *g = rtpoint_as_rtgeom(ctx, p);
    assert(g != NULL);

    /* Serialize to HEX-WKB, ISO variant. */
    size_t hex_size = 0;
    char *hexwkb = rtgeom_to_hexwkb(ctx, g, RTWKB_ISO, &hex_size);
    assert(hexwkb != NULL);
    assert(hex_size > 0);
    /* HEX-WKB must be ASCII hex — non-empty, even length. */
    assert(strlen(hexwkb) > 0);
    assert((strlen(hexwkb) % 2) == 0);

    /* Parse it back. */
    RTGEOM *g2 = rtgeom_from_hexwkb(ctx, hexwkb, 0 /* no check flags */);
    assert(g2 != NULL);

    uint32_t type = rtgeom_get_type(ctx, g2);
    assert(type == RTPOINTTYPE);

    /* Clean up. */
    rtfree(ctx, hexwkb);
    rtgeom_free(ctx, g);  /* frees p too — g is just a cast */
    rtgeom_free(ctx, g2);
    rtgeom_finish(ctx);

    printf("librttopo WKB round-trip OK\n");
    return 0;
}
