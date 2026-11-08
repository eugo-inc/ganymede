/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for graphite2: core API; calls gr_engine_version().
 */

#include <graphite2/Font.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int major = -1, minor = -1, bugfix = -1;
    gr_engine_version(&major, &minor, &bugfix);

    /* Sanity: gr_engine_version is documented to populate all three out-params with
     * non-negative integers, and the engine major version must be >= 1 (the ABI baseline). */
    assert(major >= 1);
    assert(minor >= 0);
    assert(bugfix >= 0);

    assert(major == GR2_VERSION_MAJOR);

    printf("graphite2 test OK: %d.%d.%d\n", major, minor, bugfix);
    return 0;
}
