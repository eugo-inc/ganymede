/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openexr: compression; calls exr_compression_lines_per_chunk(), exr_get_default_zip_compression_level(), exr_set_default_zip_compression_level().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openexr.h>
#include <openexr_compression.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int zip_lines = exr_compression_lines_per_chunk(EXR_COMPRESSION_ZIP);
    int zips_lines = exr_compression_lines_per_chunk(EXR_COMPRESSION_ZIPS);
    int none_lines = exr_compression_lines_per_chunk(EXR_COMPRESSION_NONE);

    assert(zip_lines == 16);
    assert(zips_lines == 1);
    assert(none_lines == 1);

    /* 2. An out-of-range codec must return a negative sentinel — proves the
     *    table bound is enforced, not silently OOB-read. */
    int bogus_lines = exr_compression_lines_per_chunk((exr_compression_t)127);
    assert(bogus_lines < 0);

    int default_level = -999;
    exr_get_default_zip_compression_level(&default_level);
    assert(default_level >= -1);
    assert(default_level <= 12);

    int saved = default_level;

    exr_set_default_zip_compression_level(9);

    int after = -999;
    exr_get_default_zip_compression_level(&after);
    assert(after == 9);

    exr_set_default_zip_compression_level(saved);

    printf("openexr compression test OK: zip=%d zips=%d default_level=%d\n",
           zip_lines, zips_lines, default_level);
    return 0;
}
