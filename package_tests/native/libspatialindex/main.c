/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libspatialindex: core API; calls IndexProperty_Create(), IndexProperty_SetIndexType(), IndexProperty_SetIndexStorage(), IndexProperty_SetIndexVariant().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

/* The libspatialindex 1.9.x C API header uses size_t and int64_t without including these. */
#include <stddef.h>
#include <stdint.h>
#include <spatialindex/capi/sidx_api.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    IndexPropertyH props = IndexProperty_Create();
    assert(props != NULL);
    assert(IndexProperty_SetIndexType(props, RT_RTree) == RT_None);
    assert(IndexProperty_SetIndexStorage(props, RT_Memory) == RT_None);
    assert(IndexProperty_SetIndexVariant(props, RT_Star) == RT_None);
    assert(IndexProperty_SetDimension(props, 2) == RT_None);

    IndexH idx = Index_Create(props);
    assert(idx != NULL);
    assert(Index_IsValid(idx) == 1);

    /* Insert two non-overlapping unit squares: [0,1]x[0,1] (id=1) and [10,11]x[10,11] (id=2). */
    double min1[2] = {0.0, 0.0}, max1[2] = {1.0, 1.0};
    double min2[2] = {10.0, 10.0}, max2[2] = {11.0, 11.0};
    assert(Index_InsertData(idx, 1, min1, max1, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 2, min2, max2, 2, NULL, 0) == RT_None);

    /* Query the rectangle [0.5,0.5]x[0.9,0.9]: should hit id=1 only. */
    double qmin[2] = {0.5, 0.5}, qmax[2] = {0.9, 0.9};
    uint64_t n = 0;
    assert(Index_Intersects_count(idx, qmin, qmax, 2, &n) == RT_None);
    assert(n == 1);

    /* Query a bbox that covers both: should hit 2. */
    double qmin_all[2] = {-1.0, -1.0}, qmax_all[2] = {12.0, 12.0};
    assert(Index_Intersects_count(idx, qmin_all, qmax_all, 2, &n) == RT_None);
    assert(n == 2);

    Index_Destroy(idx);
    IndexProperty_Destroy(props);

    /* SIDX_Version() returns a heap-allocated string; free with Index_Free per the C API contract. */
    char *version = SIDX_Version();
    assert(version != NULL);
    printf("libspatialindex test OK: %s\n", version);
    Index_Free(version);

    return 0;
}
