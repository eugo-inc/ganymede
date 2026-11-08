/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_mqtt: topic edge cases; calls aws_default_allocator(), aws_mqtt_library_init(), aws_byte_cursor_from_c_str(), aws_mqtt_is_valid_topic().
 */

#include <aws/mqtt/mqtt.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_mqtt_library_init(alloc);

    /* (1) Empty topic — must fail BOTH validators. */
    struct aws_byte_cursor empty = aws_byte_cursor_from_c_str("");
    assert(!aws_mqtt_is_valid_topic(&empty));
    assert(!aws_mqtt_is_valid_topic_filter(&empty));

    /* (2) Single-level wildcard '+' — must reject as topic, accept as filter. */
    struct aws_byte_cursor plus = aws_byte_cursor_from_c_str("demo/+/logs");
    assert(!aws_mqtt_is_valid_topic(&plus));
    assert(aws_mqtt_is_valid_topic_filter(&plus));

    /* (3) '+' embedded mid-segment — must reject as a filter (wildcard must
     *     occupy the entire level). */
    struct aws_byte_cursor bad_plus = aws_byte_cursor_from_c_str("demo/+foo/logs");
    assert(!aws_mqtt_is_valid_topic_filter(&bad_plus));

    /* (4) '$SYS/...' is a valid topic name (broker-reserved, but well-formed). */
    struct aws_byte_cursor sys_topic = aws_byte_cursor_from_c_str("$SYS/broker/uptime");
    assert(aws_mqtt_is_valid_topic(&sys_topic));
    assert(aws_mqtt_is_valid_topic_filter(&sys_topic));

    aws_mqtt_library_clean_up();
    printf("aws_c_mqtt topic-edge-cases OK\n");
    return 0;
}
