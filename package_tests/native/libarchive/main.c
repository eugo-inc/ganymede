/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libarchive: core API; calls archive_write_new(), archive_write_set_format_pax_restricted(), archive_write_open_memory(), archive_entry_new().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <archive.h>
#include <archive_entry.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char payload[] = "demo";
    char buf[4096];
    size_t used = 0;

    /* Write a single-entry tar (ustar/pax) into the in-memory buffer. */
    struct archive *w = archive_write_new();
    assert(w != NULL);
    assert(archive_write_set_format_pax_restricted(w) == ARCHIVE_OK);
    assert(archive_write_open_memory(w, buf, sizeof(buf), &used) == ARCHIVE_OK);

    struct archive_entry *we = archive_entry_new();
    assert(we != NULL);
    archive_entry_set_pathname(we, "hello.txt");
    archive_entry_set_size(we, (int64_t)(sizeof(payload) - 1));
    archive_entry_set_filetype(we, AE_IFREG);
    archive_entry_set_perm(we, 0644);
    assert(archive_write_header(w, we) == ARCHIVE_OK);
    assert(archive_write_data(w, payload, sizeof(payload) - 1) == (int64_t)(sizeof(payload) - 1));
    archive_entry_free(we);
    assert(archive_write_close(w) == ARCHIVE_OK);
    assert(archive_write_free(w) == ARCHIVE_OK);

    /* Read the buffer back and verify the entry. */
    struct archive *r = archive_read_new();
    assert(r != NULL);
    assert(archive_read_support_format_tar(r) == ARCHIVE_OK);
    assert(archive_read_open_memory(r, buf, used) == ARCHIVE_OK);

    struct archive_entry *re = NULL;
    assert(archive_read_next_header(r, &re) == ARCHIVE_OK);
    assert(strcmp(archive_entry_pathname(re), "hello.txt") == 0);

    char rbuf[16] = {0};
    la_ssize_t n = archive_read_data(r, rbuf, sizeof(rbuf));
    assert(n == (la_ssize_t)(sizeof(payload) - 1));
    assert(memcmp(rbuf, payload, sizeof(payload) - 1) == 0);
    assert(archive_read_next_header(r, &re) == ARCHIVE_EOF);

    assert(archive_read_free(r) == ARCHIVE_OK);

    printf("libarchive test OK: %s\n", archive_version_string());
    return 0;
}
