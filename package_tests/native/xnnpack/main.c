/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xnnpack: core API; calls xnn_initialize(), xnn_deinitialize().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* NULL allocator = use the default (malloc/free). */
    enum xnn_status init_status = xnn_initialize(NULL);
    assert(init_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    printf("xnnpack test OK\n");
    return 0;
}
