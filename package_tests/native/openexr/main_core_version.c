/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openexr: core version; calls exr_get_library_version().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openexr.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int major = -1;
    int minor = -1;
    int patch = -1;
    const char* extra = NULL;

    exr_get_library_version(&major, &minor, &patch, &extra);

    /* Versions must be populated and non-negative. */
    assert(major >= 0);
    assert(minor >= 0);
    assert(patch >= 0);

    /* The library was actually written-to (the -1 sentinels are gone). */
    assert(major + minor + patch > 0);

    /* `extra` may be empty but is never NULL per the C API contract. */
    assert(extra != NULL);

    /* NULL-out args must be tolerated (documented in base_units.cpp tests). */
    exr_get_library_version(NULL, &minor, &patch, &extra);
    exr_get_library_version(&major, NULL, &patch, &extra);
    exr_get_library_version(&major, &minor, NULL, &extra);
    exr_get_library_version(&major, &minor, &patch, NULL);

    printf("openexr core test OK: %d.%d.%d%s\n", major, minor, patch, extra);
    return 0;
}
