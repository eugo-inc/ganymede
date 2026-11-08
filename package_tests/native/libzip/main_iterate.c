/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libzip: iterate; calls mkstemps(), zip_open(), zip_source_buffer(), zip_file_add().
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
    char path[] = "/tmp/demo_libzip_iter_XXXXXX.zip";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    int err = 0;
    zip_t *za = zip_open(path, ZIP_CREATE | ZIP_EXCL, &err);
    assert(za != NULL);

    static const char p0[] = "first";
    static const char p1[] = "second";
    static const char p2[] = "third";

    zip_source_t *s0 = zip_source_buffer(za, p0, sizeof(p0) - 1, 0);
    zip_source_t *s1 = zip_source_buffer(za, p1, sizeof(p1) - 1, 0);
    zip_source_t *s2 = zip_source_buffer(za, p2, sizeof(p2) - 1, 0);
    assert(s0 && s1 && s2);

    assert(zip_file_add(za, "a.txt", s0, ZIP_FL_OVERWRITE) >= 0);
    assert(zip_file_add(za, "b.txt", s1, ZIP_FL_OVERWRITE) >= 0);
    assert(zip_file_add(za, "c.txt", s2, ZIP_FL_OVERWRITE) >= 0);
    assert(zip_close(za) == 0);

    /* Reopen rw to mutate. */
    za = zip_open(path, 0, &err);
    assert(za != NULL);
    assert(zip_get_num_entries(za, 0) == 3);

    /* Rename b.txt -> renamed.txt. */
    zip_int64_t i_b = zip_name_locate(za, "b.txt", 0);
    assert(i_b >= 0);
    assert(zip_file_rename(za, (zip_uint64_t)i_b, "renamed.txt", 0) == 0);

    /* Delete c.txt. */
    zip_int64_t i_c = zip_name_locate(za, "c.txt", 0);
    assert(i_c >= 0);
    assert(zip_delete(za, (zip_uint64_t)i_c) == 0);

    assert(zip_close(za) == 0);

    /* Reopen read-only and walk all entries. */
    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);
    zip_int64_t n = zip_get_num_entries(za, 0);
    assert(n == 2);

    int saw_a = 0, saw_renamed = 0, saw_c = 0;
    for (zip_int64_t i = 0; i < n; ++i) {
        const char *nm = zip_get_name(za, (zip_uint64_t)i, 0);
        assert(nm != NULL);
        if (strcmp(nm, "a.txt") == 0) saw_a = 1;
        else if (strcmp(nm, "renamed.txt") == 0) saw_renamed = 1;
        else if (strcmp(nm, "c.txt") == 0) saw_c = 1;
    }
    assert(saw_a == 1);
    assert(saw_renamed == 1);
    assert(saw_c == 0);  /* deleted */

    /* Original name must no longer resolve. */
    assert(zip_name_locate(za, "b.txt", 0) == -1);
    assert(zip_name_locate(za, "c.txt", 0) == -1);
    assert(zip_name_locate(za, "renamed.txt", 0) >= 0);

    /* Sanity-check the renamed payload survived the directory rewrite. */
    zip_file_t *zf = zip_fopen(za, "renamed.txt", 0);
    assert(zf != NULL);
    char buf[16] = {0};
    zip_int64_t r = zip_fread(zf, buf, sizeof(buf) - 1);
    assert(r == (zip_int64_t)(sizeof(p1) - 1));
    assert(memcmp(buf, p1, sizeof(p1) - 1) == 0);
    zip_fclose(zf);

    zip_close(za);
    unlink(path);

    printf("libzip rename/delete/iterate test OK\n");
    return 0;
}
