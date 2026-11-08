/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xnnpack: runtime; calls xnn_initialize(), xnn_create_subgraph(), xnn_define_tensor_value(), xnn_create_workspace().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    enum xnn_status init_status = xnn_initialize(NULL);
    assert(init_status == xnn_status_success);

    xnn_subgraph_t subgraph = NULL;
    enum xnn_status sg_status = xnn_create_subgraph(
        /*external_value_ids=*/1,
        /*flags=*/0,
        &subgraph);
    assert(sg_status == xnn_status_success);
    assert(subgraph != NULL);

    /* Define a single 4-element fp32 external input/output. */
    const size_t dims[] = {4};
    uint32_t value_id = XNN_INVALID_VALUE_ID;
    enum xnn_status val_status = xnn_define_tensor_value(
        subgraph,
        xnn_datatype_fp32,
        /*num_dims=*/1,
        dims,
        /*data=*/NULL,
        /*external_id=*/0,
        XNN_VALUE_FLAG_EXTERNAL_INPUT,
        &value_id);
    assert(val_status == xnn_status_success);
    assert(value_id == 0);

    xnn_workspace_t workspace = NULL;
    enum xnn_status ws_status = xnn_create_workspace(&workspace);
    assert(ws_status == xnn_status_success);
    assert(workspace != NULL);

    xnn_runtime_t runtime = NULL;
    enum xnn_status rt_status = xnn_create_runtime_v4(
        subgraph,
        /*weights_cache=*/NULL,
        workspace,
        /*threadpool=*/NULL,
        /*flags=*/0,
        &runtime);
    assert(rt_status == xnn_status_success);
    assert(runtime != NULL);

    enum xnn_status rt_delete_status = xnn_delete_runtime(runtime);
    assert(rt_delete_status == xnn_status_success);

    enum xnn_status ws_release_status = xnn_release_workspace(workspace);
    assert(ws_release_status == xnn_status_success);

    enum xnn_status sg_delete_status = xnn_delete_subgraph(subgraph);
    assert(sg_delete_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    printf("xnnpack runtime test OK\n");
    return 0;
}
