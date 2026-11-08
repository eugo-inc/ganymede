/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libbson: core API; calls bson_get_version().
 */

#include <bson/bson.h>
#include <stdio.h>

int main(void) {
    /* Header sanity + version macro. */
    printf("libbson test OK: %s\n", bson_get_version());
    return 0;
}
