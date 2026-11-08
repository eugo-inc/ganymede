/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libev: core API; calls ev_version_major(), ev_version_minor().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <ev.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Check that the library version functions return sane values: the library's major
     * version matches the header's, and its minor version is not older (the compatibility
     * check the libev manual recommends). */
    int v_major = ev_version_major();
    int v_minor = ev_version_minor();
    assert(v_major == EV_VERSION_MAJOR);
    assert(v_minor >= EV_VERSION_MINOR);
    printf("libev test OK: %d.%d\n", v_major, v_minor);
    return 0;
}
