/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for proj: operations; calls proj_context_create(), proj_create_from_database(), proj_create_operation_factory_context(), proj_create_operations().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <proj.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    PJ_CONTEXT *ctx = proj_context_create();
    assert(ctx != NULL);

    PJ *src = proj_create_from_database(ctx, "EPSG", "4326",
                                        PJ_CATEGORY_CRS, 0, NULL);
    assert(src != NULL);
    PJ *dst = proj_create_from_database(ctx, "EPSG", "32633",
                                        PJ_CATEGORY_CRS, 0, NULL);
    assert(dst != NULL);

    PJ_OPERATION_FACTORY_CONTEXT *opctx =
        proj_create_operation_factory_context(ctx, NULL);
    assert(opctx != NULL);

    PJ_OBJ_LIST *ops = proj_create_operations(ctx, src, dst, opctx);
    assert(ops != NULL);

    int n = proj_list_get_count(ops);
    assert(n >= 1); /* at least one candidate operation must exist */

    PJ *op = proj_list_get(ctx, ops, 0);
    assert(op != NULL);

    PJ *op_norm = proj_normalize_for_visualization(ctx, op);
    assert(op_norm != NULL);

    /* Rome (lon=12.4964, lat=41.9028) — well inside UTM zone 33N (9°E..15°E). */
    PJ_COORD rome = proj_coord(12.4964, 41.9028, 0.0, 0.0);
    PJ_COORD projected = proj_trans(op_norm, PJ_FWD, rome);
    assert(isfinite(projected.xy.x));
    assert(isfinite(projected.xy.y));
    /* UTM easting must be inside the (~167000, ~833000) zone-width envelope. */
    assert(projected.xy.x > 100000.0);
    assert(projected.xy.x < 900000.0);
    /* UTM northing in the northern hemisphere is the meters from the equator;
     * Rome at ~42°N sits around 4.64e6 m. */
    assert(projected.xy.y > 4.0e6);
    assert(projected.xy.y < 5.5e6);

    proj_destroy(op_norm);
    proj_destroy(op);
    proj_list_destroy(ops);
    proj_operation_factory_context_destroy(opctx);
    proj_destroy(dst);
    proj_destroy(src);
    proj_context_destroy(ctx);

    printf("proj operation factory OK: %d candidate(s)\n", n);
    return 0;
}
