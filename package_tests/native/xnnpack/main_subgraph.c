/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xnnpack: subgraph; calls xnn_initialize(), xnn_create_subgraph(), xnn_define_tensor_value(), xnn_delete_subgraph().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

int main(void) {
    enum xnn_status init_status = xnn_initialize(NULL);
    assert(init_status == xnn_status_success);

    xnn_subgraph_t subgraph = NULL;
    enum xnn_status create_status = xnn_create_subgraph(
        /*external_value_ids=*/2,
        /*flags=*/0,
        &subgraph);
    assert(create_status == xnn_status_success);
    assert(subgraph != NULL);

    /* Define a 2x3 fp32 external-input tensor; external_id=0 falls inside the
     * 2 reserved IDs. id_out must be written and equal external_id on success. */
    const size_t dims[] = {2, 3};
    uint32_t value_id = XNN_INVALID_VALUE_ID;
    enum xnn_status define_status = xnn_define_tensor_value(
        subgraph,
        xnn_datatype_fp32,
        /*num_dims=*/2,
        dims,
        /*data=*/NULL,
        /*external_id=*/0,
        XNN_VALUE_FLAG_EXTERNAL_INPUT,
        &value_id);
    assert(define_status == xnn_status_success);
    assert(value_id == 0);

    enum xnn_status delete_status = xnn_delete_subgraph(subgraph);
    assert(delete_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    printf("xnnpack subgraph test OK\n");
    return 0;
}
