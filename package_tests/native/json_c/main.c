/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for json_c: core API; calls json_c_version().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json-c/json.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *v = json_c_version();
    assert(v != NULL);
    assert(strlen(v) > 0);
    printf("json_c test OK: %s\n", v);
    return 0;
}
