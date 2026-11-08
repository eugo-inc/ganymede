/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_iot: errors; calls aws_default_allocator(), aws_iotdevice_library_init(), aws_error_name(), aws_iotdevice_library_clean_up().
 */

#include <aws/iotdevice/iotdevice.h>
#include <aws/common/allocator.h>
#include <aws/common/common.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_iotdevice_library_init(alloc);

    /* Two IoT-specific error codes, one from each major subsystem (defender, secure
     * tunneling). aws_error_name() must return non-NULL and non-"Unknown Error Code"
     * for both — otherwise the error-info registration is broken. */
    const char *defender_name = aws_error_name(AWS_ERROR_IOTDEVICE_DEFENDER_INVALID_REPORT_INTERVAL);
    const char *tunnel_name = aws_error_name(AWS_ERROR_IOTDEVICE_SECURE_TUNNELING_INVALID_STREAM_ID);

    assert(defender_name != NULL);
    assert(tunnel_name != NULL);

    /* "Unknown Error Code" is what aws-c-common falls back to when no registration
     * was found for the given code. Catching that string explicitly proves the
     * registration actually wired the IoT-package range into the global table. */
    assert(strstr(defender_name, "Unknown") == NULL);
    assert(strstr(tunnel_name, "Unknown") == NULL);

    /* Sanity: the two distinct error codes should map to distinct names. */
    assert(strcmp(defender_name, tunnel_name) != 0);

    aws_iotdevice_library_clean_up();

    printf("aws_c_iot errors test OK: defender=%s, tunnel=%s\n", defender_name, tunnel_name);
    return 0;
}
