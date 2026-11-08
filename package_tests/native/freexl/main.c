/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freexl: core API; calls freexl_version(), freexl_open().
 */

#include <freexl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *v = freexl_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    /* freexl_open() on a path that does not exist must NOT return FREEXL_OK.
     * This proves the symbol is linkable + the lib executes its error path. */
    const void *handle = NULL;
    int rc = freexl_open("/nonexistent/path/__demo_freexl_smoke.xls", &handle);
    assert(rc != FREEXL_OK);

    printf("freexl test OK: %s\n", v);
    return 0;
}
