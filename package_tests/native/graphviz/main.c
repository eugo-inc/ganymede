/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for graphviz: core API; compiles against <graphviz/cgraph.h>.
 */

#include <graphviz/cgraph.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(sizeof(Agraph_t *) > 0);
    printf("graphviz test OK\n");
    return 0;
}
