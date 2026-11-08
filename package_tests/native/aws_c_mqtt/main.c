/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_mqtt: core API; calls aws_default_allocator(), aws_mqtt_library_init(), aws_byte_cursor_from_c_str(), aws_mqtt_is_valid_topic().
 */

#include <aws/mqtt/mqtt.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_mqtt_library_init(alloc);

    struct aws_byte_cursor plain = aws_byte_cursor_from_c_str("demo/healthz");
    assert(aws_mqtt_is_valid_topic(&plain));
    assert(aws_mqtt_is_valid_topic_filter(&plain));

    struct aws_byte_cursor wildcard = aws_byte_cursor_from_c_str("demo/#");
    assert(!aws_mqtt_is_valid_topic(&wildcard));
    assert(aws_mqtt_is_valid_topic_filter(&wildcard));

    /* aws_error_str must return a non-NULL human-readable string for the MQTT
     * sentinel error code — catches a missing mqtt error-table registration. */
    const char *err = aws_error_str(AWS_ERROR_MQTT_TIMEOUT);
    assert(err != NULL);
    assert(strlen(err) > 0);

    aws_mqtt_library_clean_up();

    printf("aws_c_mqtt test OK: error_str(MQTT_TIMEOUT)=\"%s\"\n", err);
    return 0;
}
