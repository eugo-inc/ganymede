/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_iot: defender setters; calls aws_default_allocator(), aws_iotdevice_library_init(), aws_byte_cursor_from_c_str(), aws_iotdevice_defender_config_create().
 */

#include <aws/iotdevice/iotdevice.h>
#include <aws/iotdevice/device_defender.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/common.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>

static int sample_number_metric_fn(double *value, void *userdata) {
    (void)userdata;
    *value = 42.0;
    return AWS_OP_SUCCESS;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_iotdevice_library_init(alloc);

    struct aws_byte_cursor thing_name = aws_byte_cursor_from_c_str("demo-setter-probe");
    struct aws_iotdevice_defender_task_config *config = NULL;
    /* aws_iotdevice_defender_config_create accepts only AWS_IDDRF_JSON; SHORT_JSON and
     * CBOR are declared in the enum but not implemented, so JSON is used to exercise
     * the setter API. */
    int rc = aws_iotdevice_defender_config_create(&config, alloc, &thing_name, AWS_IDDRF_JSON);
    assert(rc == AWS_OP_SUCCESS);
    assert(config != NULL);

    /* Task period: a positive ns value should be accepted. */
    rc = aws_iotdevice_defender_config_set_task_period_ns(config, 5ULL * 60ULL * 1000000000ULL);
    assert(rc == AWS_OP_SUCCESS);

    /* Callback userdata: any non-NULL pointer is fine — the setter must accept it. */
    int sentinel = 0xC0FFEE;
    rc = aws_iotdevice_defender_config_set_callback_userdata(config, &sentinel);
    assert(rc == AWS_OP_SUCCESS);

    /* Custom number metric: void return, but must not crash and must accept a
     * well-formed metric name + supplier. */
    struct aws_byte_cursor metric_name = aws_byte_cursor_from_c_str("demo_test_metric");
    aws_iotdevice_defender_config_register_number_metric(config, &metric_name, sample_number_metric_fn, NULL);

    aws_iotdevice_defender_config_clean_up(config);
    aws_iotdevice_library_clean_up();

    printf("aws_c_iot defender-setters test OK\n");
    return 0;
}
