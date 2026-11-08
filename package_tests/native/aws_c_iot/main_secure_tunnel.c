/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_iot: secure tunnel; calls aws_default_allocator(), aws_iotdevice_library_init(), aws_iotdevice_library_clean_up().
 */

#include <aws/iotdevice/iotdevice.h>
#include <aws/iotdevice/secure_tunneling.h>
#include <aws/common/allocator.h>
#include <aws/common/common.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_iotdevice_library_init(alloc);

    /* Header surface — enum values from secure_tunneling.h must be visible. */
    assert(AWS_SECURE_TUNNELING_SOURCE_MODE != AWS_SECURE_TUNNELING_DESTINATION_MODE);
    assert(AWS_SECURE_TUNNEL_MT_UNKNOWN == 0);
    assert(AWS_SECURE_TUNNEL_MT_DATA == 1);
    assert(AWS_SECURE_TUNNEL_MT_STREAM_START == 2);
    assert(AWS_SECURE_TUNNEL_MT_CONNECTION_RESET == 7);

    /* Macro surface — protocol-level constant from the public header. */
    assert(AWS_IOT_ST_SPLIT_MESSAGE_SIZE > 0);
    assert(AWS_IOT_ST_SPLIT_MESSAGE_SIZE <= 65535);

    /* Link surface — take the address of the secure-tunnel constructor without
     * calling it. If the secure-tunneling .o files weren't compiled into the .so,
     * this won't link. (Cast to void* + volatile to defeat dead-code elimination
     * regardless of compiler optimization level.) */
    volatile void *fn_addr = (void *)&aws_secure_tunnel_new;
    assert(fn_addr != NULL);

    aws_iotdevice_library_clean_up();

    printf("aws_c_iot secure-tunnel test OK\n");
    return 0;
}
