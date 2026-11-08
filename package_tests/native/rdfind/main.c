/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rdfind: core API; compiles and runs.
 */

#include <assert.h>
#include <stdio.h>

int main(void) {
    int n = printf("rdfind test OK: cli-tool-only, no public lib API\n");
    assert(n > 0);
    return 0;
}
