/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libattr: get set remove; calls attr_set(), attr_get(), attr_remove().
 * Works on a scratch file in the current directory. Where the file system has no user
 * extended attributes, the calls must fail with ENOTSUP and nothing more is checked.
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
    assert(ATTR_CREATE != ATTR_REPLACE);
    assert(ATTR_MAX_VALUELEN > 0);
    /* ENOATTR must be visible (the header falls back to ENODATA on Linux). */
    assert(ENOATTR != 0);

    char path[] = "demo_libattr_get_set_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);

    const char *name = "demo_libattr_probe";
    char readback[64];
    int valuelen = (int)sizeof(readback);

    /* ATTR_CREATE: the attribute does not exist yet, so this creates it. */
    int rc = attr_set(path, name, "x", 1, ATTR_CREATE);
    if (rc != 0) {
        assert(rc == -1 && unsupported(errno));
        rc = attr_get(path, name, readback, &valuelen, 0);
        assert(rc == -1 && (unsupported(errno) || errno == ENOATTR));
        unlink(path);
        printf("libattr set/get/remove probe OK: this file system has no user extended attributes\n");
        return 0;
    }

    rc = attr_get(path, name, readback, &valuelen, 0);
    assert(rc == 0);
    assert(valuelen == 1 && readback[0] == 'x');

    /* ATTR_CREATE on an existing attribute fails with EEXIST. */
    rc = attr_set(path, name, "y", 1, ATTR_CREATE);
    assert(rc == -1 && errno == EEXIST);

    /* ATTR_REPLACE overwrites it, and attr_get reads the new value back. */
    rc = attr_set(path, name, "yz", 2, ATTR_REPLACE);
    assert(rc == 0);
    valuelen = (int)sizeof(readback);
    rc = attr_get(path, name, readback, &valuelen, 0);
    assert(rc == 0);
    assert(valuelen == 2 && memcmp(readback, "yz", 2) == 0);

    /* A buffer that is too small fails with E2BIG and reports the length needed. */
    valuelen = 1;
    rc = attr_get(path, name, readback, &valuelen, 0);
    assert(rc == -1 && errno == E2BIG);
    assert(valuelen == 2);

    rc = attr_remove(path, name, 0);
    assert(rc == 0);

    /* The attribute is gone: ENOATTR (ENODATA on Linux). */
    valuelen = (int)sizeof(readback);
    rc = attr_get(path, name, readback, &valuelen, 0);
    assert(rc == -1 && errno == ENOATTR);

    unlink(path);
    printf("libattr set/get/remove probe OK: create, replace, read back and remove\n");
    return 0;
}

#pragma GCC diagnostic pop
