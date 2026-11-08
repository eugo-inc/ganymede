/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libspatialindex: bounds; calls IndexProperty_Create(), IndexProperty_SetIndexType(), IndexProperty_SetIndexStorage(), IndexProperty_SetDimension().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

/* The libspatialindex 1.9.x C API header uses size_t and int64_t without including these. */
#include <stddef.h>
#include <stdint.h>
#include <spatialindex/capi/sidx_api.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    IndexPropertyH props = IndexProperty_Create();
    assert(props != NULL);
    assert(IndexProperty_SetIndexType(props, RT_RTree) == RT_None);
    assert(IndexProperty_SetIndexStorage(props, RT_Memory) == RT_None);
    assert(IndexProperty_SetDimension(props, 2) == RT_None);

    IndexH idx = Index_Create(props);
    assert(idx != NULL);

    /* Insert four rectangles. The global MBR should be [-3, 7] x [-2, 9]. */
    double a_min[2] = {-3.0, -2.0}, a_max[2] = {0.0, 1.0};
    double b_min[2] = { 2.0,  3.0}, b_max[2] = {4.0, 5.0};
    double c_min[2] = { 1.0, -1.0}, c_max[2] = {7.0, 2.0};
    double d_min[2] = { 0.0,  6.0}, d_max[2] = {3.0, 9.0};
    assert(Index_InsertData(idx, 10, a_min, a_max, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 11, b_min, b_max, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 12, c_min, c_max, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 13, d_min, d_max, 2, NULL, 0) == RT_None);

    double *ppdMin = NULL, *ppdMax = NULL;
    uint32_t dim = 0;
    assert(Index_GetBounds(idx, &ppdMin, &ppdMax, &dim) == RT_None);
    assert(ppdMin != NULL);
    assert(ppdMax != NULL);
    assert(dim == 2);

    /* Expected global MBR: x in [-3, 7], y in [-2, 9]. */
    assert(fabs(ppdMin[0] - (-3.0)) < 1e-9);
    assert(fabs(ppdMin[1] - (-2.0)) < 1e-9);
    assert(fabs(ppdMax[0] -   7.0)  < 1e-9);
    assert(fabs(ppdMax[1] -   9.0)  < 1e-9);

    printf("libspatialindex Bounds test OK: MBR=[%.1f,%.1f]x[%.1f,%.1f] dim=%u\n",
           ppdMin[0], ppdMax[0], ppdMin[1], ppdMax[1], dim);

    /* Per the C API contract, both buffers are caller-owned heap allocations. */
    Index_Free(ppdMin);
    Index_Free(ppdMax);

    Index_Destroy(idx);
    IndexProperty_Destroy(props);
    return 0;
}
