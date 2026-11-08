/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xnnpack: workspace; calls xnn_initialize(), xnn_create_workspace(), xnn_release_workspace(), xnn_deinitialize().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    enum xnn_status init_status = xnn_initialize(NULL);
    assert(init_status == xnn_status_success);

    xnn_workspace_t workspace = NULL;
    enum xnn_status create_status = xnn_create_workspace(&workspace);
    assert(create_status == xnn_status_success);
    assert(workspace != NULL);

    enum xnn_status release_status = xnn_release_workspace(workspace);
    assert(release_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    printf("xnnpack workspace test OK\n");
    return 0;
}
