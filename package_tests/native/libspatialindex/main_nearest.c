/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libspatialindex: nearest; calls IndexProperty_Create(), IndexProperty_SetIndexType(), IndexProperty_SetIndexStorage(), IndexProperty_SetDimension().
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
    assert(IndexProperty_SetDimension(props, 2) == RT_None);

    IndexH idx = Index_Create(props);
    assert(idx != NULL);
    assert(Index_IsValid(idx) == 1);

    /* Insert three point-rectangles at (0,0), (5,5), (10,10). */
    double p1min[2] = {0.0, 0.0},   p1max[2] = {0.0, 0.0};
    double p2min[2] = {5.0, 5.0},   p2max[2] = {5.0, 5.0};
    double p3min[2] = {10.0, 10.0}, p3max[2] = {10.0, 10.0};
    assert(Index_InsertData(idx, 1, p1min, p1max, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 2, p2min, p2max, 2, NULL, 0) == RT_None);
    assert(Index_InsertData(idx, 3, p3min, p3max, 2, NULL, 0) == RT_None);

    /* Query 1: a region that encloses only (0,0). KNN traversal returns id=1. */
    double q1min[2] = {-1.0, -1.0}, q1max[2] = {1.0, 1.0};
    int64_t *ids = NULL;
    uint64_t n = 0;
    assert(Index_NearestNeighbors_id(idx, q1min, q1max, 2, &ids, &n) == RT_None);
    assert(n == 1);
    assert(ids != NULL);
    /* The single, nearest result MUST be id=1. */
    assert(ids[0] == 1);
    printf("libspatialindex KNN test: small region -> %llu id(s), nearest=%lld\n",
           (unsigned long long)n, (long long)ids[0]);
    Index_Free(ids); /* caller-owns-the-buffer ABI */

    /* Query 2: a region enclosing (0,0) and (5,5). Ordered nearest-first: 1, 2. */
    double q2min[2] = {-1.0, -1.0}, q2max[2] = {6.0, 6.0};
    ids = NULL;
    n = 0;
    assert(Index_NearestNeighbors_id(idx, q2min, q2max, 2, &ids, &n) == RT_None);
    assert(n == 2);
    assert(ids != NULL);
    assert(ids[0] == 1); /* nearest first */
    assert(ids[1] == 2);
    Index_Free(ids);

    /* Query 3: a region enclosing all three. Ordered nearest-first: 1, 2, 3. */
    double q3min[2] = {-1.0, -1.0}, q3max[2] = {11.0, 11.0};
    ids = NULL;
    n = 0;
    assert(Index_NearestNeighbors_id(idx, q3min, q3max, 2, &ids, &n) == RT_None);
    assert(n == 3);
    assert(ids != NULL);
    assert(ids[0] == 1);
    assert(ids[1] == 2);
    assert(ids[2] == 3);
    printf("libspatialindex KNN test OK: full region -> %llu ids, order=%lld %lld %lld\n",
           (unsigned long long)n, (long long)ids[0], (long long)ids[1], (long long)ids[2]);
    Index_Free(ids);

    Index_Destroy(idx);
    IndexProperty_Destroy(props);
    return 0;
}
