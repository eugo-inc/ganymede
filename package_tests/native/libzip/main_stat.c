/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzip: stat; calls mkstemps(), zip_open(), zip_source_buffer(), zip_file_add().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zip.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libzip_stat_XXXXXX.zip";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    int err = 0;
    zip_t *za = zip_open(path, ZIP_CREATE | ZIP_EXCL, &err);
    assert(za != NULL);

    static const char payload_a[] = "alpha-payload";
    static const char payload_b[] = "bravo-payload-longer";
    const zip_uint64_t len_a = sizeof(payload_a) - 1;
    const zip_uint64_t len_b = sizeof(payload_b) - 1;

    zip_source_t *src_a = zip_source_buffer(za, payload_a, len_a, 0);
    assert(src_a != NULL);
    assert(zip_file_add(za, "alpha.txt", src_a, ZIP_FL_OVERWRITE) >= 0);

    zip_source_t *src_b = zip_source_buffer(za, payload_b, len_b, 0);
    assert(src_b != NULL);
    assert(zip_file_add(za, "subdir/bravo.txt", src_b, ZIP_FL_OVERWRITE) >= 0);

    /* Explicitly request STORE for alpha to lock the compression method in the test. */
    zip_int64_t alpha_idx = zip_name_locate(za, "alpha.txt", 0);
    assert(alpha_idx >= 0);
    assert(zip_set_file_compression(za, (zip_uint64_t)alpha_idx, ZIP_CM_STORE, 0) == 0);

    assert(zip_close(za) == 0);

    /* Reopen + introspect. */
    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);
    assert(zip_get_num_entries(za, 0) == 2);

    /* Name-locate hits for known entries... */
    zip_int64_t i_a = zip_name_locate(za, "alpha.txt", 0);
    zip_int64_t i_b = zip_name_locate(za, "subdir/bravo.txt", 0);
    assert(i_a >= 0);
    assert(i_b >= 0);
    assert(i_a != i_b);

    /* ...and misses on an unknown name (libzip sets the error + returns -1). */
    assert(zip_name_locate(za, "nope.txt", 0) == -1);

    zip_stat_t st;
    zip_stat_init(&st);
    assert(zip_stat_index(za, (zip_uint64_t)i_a, 0, &st) == 0);
    assert((st.valid & ZIP_STAT_SIZE) != 0);
    assert(st.size == len_a);
    assert((st.valid & ZIP_STAT_NAME) != 0);
    assert(st.name != NULL);
    assert(strcmp(st.name, "alpha.txt") == 0);
    assert((st.valid & ZIP_STAT_COMP_METHOD) != 0);
    assert(st.comp_method == ZIP_CM_STORE);

    zip_stat_init(&st);
    assert(zip_stat_index(za, (zip_uint64_t)i_b, 0, &st) == 0);
    assert(st.size == len_b);
    assert(strcmp(st.name, "subdir/bravo.txt") == 0);

    /* zip_get_name on a known index must match. */
    const char *nm = zip_get_name(za, (zip_uint64_t)i_a, 0);
    assert(nm != NULL);
    assert(strcmp(nm, "alpha.txt") == 0);

    zip_close(za);
    unlink(path);

    printf("libzip stat/name-locate test OK\n");
    return 0;
}
