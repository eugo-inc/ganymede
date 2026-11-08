/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_iot: core API; calls aws_default_allocator(), aws_iotdevice_library_init(), aws_byte_cursor_from_c_str(), aws_iotdevice_defender_config_create().
 */

#include <aws/iotdevice/iotdevice.h>
#include <aws/iotdevice/device_defender.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/common.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_iotdevice_library_init(alloc);

    struct aws_byte_cursor thing_name = aws_byte_cursor_from_c_str("demo-smoke");
    struct aws_iotdevice_defender_task_config *config = NULL;
    int rc = aws_iotdevice_defender_config_create(&config, alloc, &thing_name, AWS_IDDRF_JSON);
    assert(rc == AWS_OP_SUCCESS);
    assert(config != NULL);

    aws_iotdevice_defender_config_clean_up(config);

    aws_iotdevice_library_clean_up();

    printf("aws_c_iot test OK\n");
    return 0;
}
