/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libspatialindex: property rt; calls IndexProperty_Create(), IndexProperty_SetIndexType(), IndexProperty_GetIndexType(), IndexProperty_SetIndexStorage().
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
    assert(IndexProperty_GetIndexType(props) == RT_RTree);

    assert(IndexProperty_SetIndexStorage(props, RT_Memory) == RT_None);
    assert(IndexProperty_GetIndexStorage(props) == RT_Memory);

    assert(IndexProperty_SetIndexVariant(props, RT_Star) == RT_None);
    assert(IndexProperty_GetIndexVariant(props) == RT_Star);

    assert(IndexProperty_SetDimension(props, 3) == RT_None);
    assert(IndexProperty_GetDimension(props) == 3);

    assert(IndexProperty_SetFillFactor(props, 0.75) == RT_None);
    double ff = IndexProperty_GetFillFactor(props);
    assert(ff > 0.0 && ff <= 1.0);
    assert(fabs(ff - 0.75) < 1e-9);

    IndexH idx = Index_Create(props);
    assert(idx != NULL);
    assert(Index_IsValid(idx) == 1);

    IndexPropertyH recovered = Index_GetProperties(idx);
    assert(recovered != NULL);
    assert(IndexProperty_GetDimension(recovered) == 3);
    assert(IndexProperty_GetIndexType(recovered) == RT_RTree);

    printf("libspatialindex property round-trip OK: type=%d storage=%d "
           "variant=%d dim=%u fill=%.2f\n",
           (int)IndexProperty_GetIndexType(props),
           (int)IndexProperty_GetIndexStorage(props),
           (int)IndexProperty_GetIndexVariant(props),
           IndexProperty_GetDimension(props),
           ff);

    /* Index_GetProperties returns a handle the caller owns per the C API contract. */
    IndexProperty_Destroy(recovered);
    Index_Destroy(idx);
    IndexProperty_Destroy(props);
    return 0;
}
