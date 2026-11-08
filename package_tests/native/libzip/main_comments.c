/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzip: comments; calls mkstemps(), zip_open(), zip_source_buffer(), zip_file_add().
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
    char path[] = "/tmp/demo_libzip_comments_XXXXXX.zip";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    int err = 0;
    zip_t *za = zip_open(path, ZIP_CREATE | ZIP_EXCL, &err);
    assert(za != NULL);

    static const char payload[] = "commented-entry-payload";
    const zip_uint64_t payload_len = sizeof(payload) - 1;

    zip_source_t *src = zip_source_buffer(za, payload, payload_len, 0);
    assert(src != NULL);
    zip_int64_t idx = zip_file_add(za, "noted.txt", src, ZIP_FL_OVERWRITE);
    assert(idx >= 0);

    static const char arc_comment[] = "Demo libzip archive-level comment";
    static const char entry_comment[] = "per-entry comment";
    const zip_uint16_t arc_len = (zip_uint16_t)(sizeof(arc_comment) - 1);
    const zip_uint16_t entry_len = (zip_uint16_t)(sizeof(entry_comment) - 1);

    assert(zip_set_archive_comment(za, arc_comment, arc_len) == 0);
    assert(zip_file_set_comment(za, (zip_uint64_t)idx, entry_comment, entry_len, 0) == 0);

    assert(zip_close(za) == 0);

    /* Reopen read-only and inspect both comments. */
    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);

    int arc_got_len = 0;
    const char *arc_got = zip_get_archive_comment(za, &arc_got_len, 0);
    assert(arc_got != NULL);
    assert(arc_got_len == (int)arc_len);
    assert(memcmp(arc_got, arc_comment, arc_len) == 0);

    /* Per-entry comment: zip_name_locate then zip_file_get_comment. */
    zip_int64_t i_noted = zip_name_locate(za, "noted.txt", 0);
    assert(i_noted >= 0);

    zip_uint32_t entry_got_len = 0;
    const char *entry_got = zip_file_get_comment(za, (zip_uint64_t)i_noted, &entry_got_len, 0);
    assert(entry_got != NULL);
    assert(entry_got_len == (zip_uint32_t)entry_len);
    assert(memcmp(entry_got, entry_comment, entry_len) == 0);

    zip_close(za);
    /* Reopen rw to add a second entry without a comment. */
    za = zip_open(path, 0, &err);
    assert(za != NULL);
    static const char p2[] = "uncommented";
    zip_source_t *src2 = zip_source_buffer(za, p2, sizeof(p2) - 1, 0);
    assert(src2 != NULL);
    assert(zip_file_add(za, "plain.txt", src2, ZIP_FL_OVERWRITE) >= 0);
    assert(zip_close(za) == 0);

    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);
    zip_int64_t i_plain = zip_name_locate(za, "plain.txt", 0);
    assert(i_plain >= 0);

    zip_uint32_t plain_len = 99;  /* sentinel; libzip must overwrite */
    const char *plain_got = zip_file_get_comment(za, (zip_uint64_t)i_plain, &plain_len, 0);
    /* libzip returns a non-NULL pointer with length 0 for a missing comment. */
    assert(plain_got != NULL);
    assert(plain_len == 0);

    zip_close(za);
    unlink(path);

    printf("libzip comments test OK\n");
    return 0;
}
