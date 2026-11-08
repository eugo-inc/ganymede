/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libarchive: zip; calls archive_write_new(), archive_write_set_format_zip(), archive_write_open_memory(), archive_entry_new().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <archive.h>
#include <archive_entry.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char p1[] = "alpha-payload-demo-libarchive-zip-test";
    static const char p2[] = "beta";
    char buf[8192];
    size_t used = 0;

    /* Write a 2-entry zip with default (deflate) compression. */
    struct archive *w = archive_write_new();
    assert(w != NULL);
    assert(archive_write_set_format_zip(w) == ARCHIVE_OK);
    assert(archive_write_open_memory(w, buf, sizeof(buf), &used) == ARCHIVE_OK);

    struct archive_entry *e1 = archive_entry_new();
    assert(e1 != NULL);
    archive_entry_set_pathname(e1, "dir/alpha.txt");
    archive_entry_set_size(e1, (int64_t)(sizeof(p1) - 1));
    archive_entry_set_filetype(e1, AE_IFREG);
    archive_entry_set_perm(e1, 0644);
    assert(archive_write_header(w, e1) == ARCHIVE_OK);
    assert(archive_write_data(w, p1, sizeof(p1) - 1) == (int64_t)(sizeof(p1) - 1));
    archive_entry_free(e1);

    struct archive_entry *e2 = archive_entry_new();
    assert(e2 != NULL);
    archive_entry_set_pathname(e2, "beta.txt");
    archive_entry_set_size(e2, (int64_t)(sizeof(p2) - 1));
    archive_entry_set_filetype(e2, AE_IFREG);
    archive_entry_set_perm(e2, 0644);
    assert(archive_write_header(w, e2) == ARCHIVE_OK);
    assert(archive_write_data(w, p2, sizeof(p2) - 1) == (int64_t)(sizeof(p2) - 1));
    archive_entry_free(e2);

    assert(archive_write_close(w) == ARCHIVE_OK);
    assert(archive_write_free(w) == ARCHIVE_OK);
    assert(used > 0);
    assert(used <= sizeof(buf));

    struct archive *r = archive_read_new();
    assert(r != NULL);
    assert(archive_read_support_format_all(r) == ARCHIVE_OK);
    assert(archive_read_support_filter_all(r) == ARCHIVE_OK);
    assert(archive_read_open_memory(r, buf, used) == ARCHIVE_OK);

    /* Entry 1. */
    struct archive_entry *re = NULL;
    assert(archive_read_next_header(r, &re) == ARCHIVE_OK);
    assert(strcmp(archive_entry_pathname(re), "dir/alpha.txt") == 0);
    char rbuf1[64] = {0};
    la_ssize_t n1 = archive_read_data(r, rbuf1, sizeof(rbuf1));
    assert(n1 == (la_ssize_t)(sizeof(p1) - 1));
    assert(memcmp(rbuf1, p1, sizeof(p1) - 1) == 0);

    /* Entry 2. */
    assert(archive_read_next_header(r, &re) == ARCHIVE_OK);
    assert(strcmp(archive_entry_pathname(re), "beta.txt") == 0);
    char rbuf2[16] = {0};
    la_ssize_t n2 = archive_read_data(r, rbuf2, sizeof(rbuf2));
    assert(n2 == (la_ssize_t)(sizeof(p2) - 1));
    assert(memcmp(rbuf2, p2, sizeof(p2) - 1) == 0);

    assert(archive_read_next_header(r, &re) == ARCHIVE_EOF);

    int fmt = archive_format(r);
    assert((fmt & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_ZIP);

    assert(archive_read_free(r) == ARCHIVE_OK);

    printf("libarchive zip test OK: archive_format=0x%x used=%zu\n", fmt, used);
    return 0;
}
