/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libarchive: version; calls archive_version_number(), archive_version_string(), archive_version_details(), archive_zlib_version().
 * The bzip2, lzma, lz4 and zstd back ends are optional build features, so their versions
 * are printed when present and never required.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <archive.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static const char *or_absent(const char *version) {
    return (version != NULL && version[0] != '\0') ? version : "(not built in)";
}

int main(void) {
    /* Basic version sanity. */
    int n = archive_version_number();
    assert(n >= 3000000);  /* libarchive 3.x or newer */

    const char *vs = archive_version_string();
    assert(vs != NULL);
    assert(strstr(vs, "libarchive") != NULL);

    const char *details = archive_version_details();
    assert(details != NULL);
    assert(strlen(details) > 0);

    /* zlib backs gzip and zip deflate support, which main_zip.c also relies on. */
    const char *zlib_v = archive_zlib_version();
    assert(zlib_v != NULL);
    assert(strlen(zlib_v) > 0);

    /* Optional back ends: each accessor returns NULL when libarchive was built without it. */
    const char *bzlib_v = or_absent(archive_bzlib_version());
    const char *lzma_v = or_absent(archive_liblzma_version());
    const char *lz4_v = or_absent(archive_liblz4_version());
    const char *zstd_v = or_absent(archive_libzstd_version());

    printf("libarchive version test OK: %s\n  details: %s\n", vs, details);
    printf("  codecs: zlib=%s bzlib=%s lzma=%s lz4=%s zstd=%s\n",
           zlib_v, bzlib_v, lzma_v, lz4_v, zstd_v);
    return 0;
}
