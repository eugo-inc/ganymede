/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libattr: core API; calls attr_list().
 * Works on a scratch file in the current directory. Where the file system has no user
 * extended attributes (attr_set() fails with ENOTSUP), only the first attr_list() call is
 * checked: it must return an empty list or fail with ENOTSUP.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <attr/attributes.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static int unsupported(int error) {
    return error == ENOTSUP || error == EOPNOTSUPP;
}

int main(void) {
    /* Flag macros from <attr/attributes.h> must be defined (a compile-time check). */
    int flags = ATTR_ROOT | ATTR_DONTFOLLOW;
    (void)flags;

    char path[] = "demo_libattr_list_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);

    union {
        attrlist_t list;
        char bytes[1024];
    } buf;
    attrlist_cursor_t cursor;

    /* A new file has no user attributes: an empty list, or ENOTSUP. */
    memset(&cursor, 0, sizeof(cursor));
    int rc = attr_list(path, buf.bytes, (int)sizeof(buf.bytes), 0, &cursor);
    assert(rc == 0 || (rc == -1 && unsupported(errno)));
    if (rc == 0) {
        assert(buf.list.al_count == 0);
        assert(buf.list.al_more == 0);
    }

    static const char value[] = "abc";
    rc = attr_set(path, "demo_libattr_probe", value, (int)(sizeof(value) - 1), 0);
    if (rc != 0) {
        assert(rc == -1 && unsupported(errno));
        unlink(path);
        printf("libattr test OK: attr_list checked on a file system without user extended attributes\n");
        return 0;
    }

    /* With one attribute set, the listing has exactly that entry and its value length. */
    memset(&cursor, 0, sizeof(cursor));
    rc = attr_list(path, buf.bytes, (int)sizeof(buf.bytes), 0, &cursor);
    assert(rc == 0);
    assert(buf.list.al_count == 1);
    assert(buf.list.al_more == 0);
    attrlist_ent_t *entry = ATTR_ENTRY(buf.bytes, 0);
    assert(strcmp(entry->a_name, "demo_libattr_probe") == 0);
    assert(entry->a_valuelen == sizeof(value) - 1);

    unlink(path);
    printf("libattr test OK: attr_list returned the one attribute set on a scratch file\n");
    return 0;
}

#pragma GCC diagnostic pop
