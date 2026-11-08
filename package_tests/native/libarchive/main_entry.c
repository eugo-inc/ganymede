/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libarchive: entry; calls archive_write_new(), archive_write_set_format_pax_restricted(), archive_write_open_memory(), archive_entry_new().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <archive.h>
#include <archive_entry.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    char buf[4096];
    size_t used = 0;

    struct archive *w = archive_write_new();
    assert(w != NULL);
    assert(archive_write_set_format_pax_restricted(w) == ARCHIVE_OK);
    assert(archive_write_open_memory(w, buf, sizeof(buf), &used) == ARCHIVE_OK);

    struct archive_entry *we = archive_entry_new();
    assert(we != NULL);
    archive_entry_set_pathname(we, "link/to/target");
    archive_entry_set_symlink(we, "../actual/target");
    archive_entry_set_filetype(we, AE_IFLNK);
    archive_entry_set_perm(we, 0777);
    archive_entry_set_uid(we, 1234);
    archive_entry_set_gid(we, 5678);
    archive_entry_set_mtime(we, 1700000000, 0);
    archive_entry_set_size(we, 0);

    assert(strcmp(archive_entry_pathname(we), "link/to/target") == 0);
    assert(strcmp(archive_entry_symlink(we), "../actual/target") == 0);
    assert(archive_entry_filetype(we) == AE_IFLNK);
    assert(archive_entry_uid(we) == 1234);
    assert(archive_entry_gid(we) == 5678);
    assert(archive_entry_mtime(we) == 1700000000);
    assert((archive_entry_perm(we) & 0777) == 0777);

    assert(archive_write_header(w, we) == ARCHIVE_OK);
    archive_entry_free(we);
    assert(archive_write_close(w) == ARCHIVE_OK);
    assert(archive_write_free(w) == ARCHIVE_OK);

    struct archive *r = archive_read_new();
    assert(r != NULL);
    assert(archive_read_support_format_tar(r) == ARCHIVE_OK);
    assert(archive_read_open_memory(r, buf, used) == ARCHIVE_OK);

    struct archive_entry *re = NULL;
    assert(archive_read_next_header(r, &re) == ARCHIVE_OK);
    assert(strcmp(archive_entry_pathname(re), "link/to/target") == 0);
    assert(strcmp(archive_entry_symlink(re), "../actual/target") == 0);
    assert(archive_entry_filetype(re) == AE_IFLNK);
    assert(archive_entry_uid(re) == 1234);
    assert(archive_entry_gid(re) == 5678);
    assert(archive_entry_mtime(re) == 1700000000);
    assert((archive_entry_perm(re) & 0777) == 0777);

    /* Clone test — archive_entry_clone must produce an independent equal entry. */
    struct archive_entry *clone = archive_entry_clone(re);
    assert(clone != NULL);
    assert(clone != re);
    assert(strcmp(archive_entry_pathname(clone), "link/to/target") == 0);
    assert(strcmp(archive_entry_symlink(clone), "../actual/target") == 0);
    assert(archive_entry_uid(clone) == 1234);
    archive_entry_free(clone);

    assert(archive_read_next_header(r, &re) == ARCHIVE_EOF);
    assert(archive_read_free(r) == ARCHIVE_OK);

    printf("libarchive entry metadata test OK\n");
    return 0;
}
