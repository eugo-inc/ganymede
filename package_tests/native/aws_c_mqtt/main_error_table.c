/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_mqtt: error table; calls aws_default_allocator(), aws_mqtt_library_init(), aws_error_str(), aws_error_name().
 */

#include <aws/mqtt/mqtt.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int is_meaningful(const char *s) {
    if (s == NULL) {
        return 0;
    }
    if (s[0] == '\0') {
        return 0;
    }
    /* aws-c-common returns "Unknown Error" for unregistered codes. */
    if (strstr(s, "Unknown Error") != NULL) {
        return 0;
    }
    return 1;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_mqtt_library_init(alloc);

    const int checked[] = {
        AWS_ERROR_MQTT_INVALID_RESERVED_BITS,
        AWS_ERROR_MQTT_INVALID_QOS,
        AWS_ERROR_MQTT_INVALID_PACKET_TYPE,
        AWS_ERROR_MQTT_INVALID_TOPIC,
        AWS_ERROR_MQTT_TIMEOUT,
        AWS_ERROR_MQTT_PROTOCOL_ERROR,
        AWS_ERROR_MQTT_NOT_CONNECTED,
        AWS_ERROR_MQTT_ALREADY_CONNECTED,
        AWS_ERROR_MQTT5_CLIENT_OPTIONS_VALIDATION,
        AWS_ERROR_MQTT5_CONNECT_OPTIONS_VALIDATION,
        AWS_ERROR_MQTT5_PUBLISH_OPTIONS_VALIDATION,
        AWS_ERROR_MQTT5_SUBSCRIBE_OPTIONS_VALIDATION,
        AWS_ERROR_MQTT5_ENCODE_FAILURE,
        AWS_ERROR_MQTT5_PING_RESPONSE_TIMEOUT,
        AWS_ERROR_MQTT5_INVALID_UTF8_STRING,
    };
    const size_t n = sizeof(checked) / sizeof(checked[0]);

    /* (1) + (2): every code must yield a meaningful string AND a non-empty name. */
    for (size_t i = 0; i < n; ++i) {
        const char *str = aws_error_str(checked[i]);
        const char *name = aws_error_name(checked[i]);
        assert(is_meaningful(str));
        assert(name != NULL && name[0] != '\0');
    }

    /* (3) The MQTT3 INVALID_TOPIC and MQTT5 INVALID_UTF8_STRING sentinels MUST
     *     have distinct human strings — catches a stub table that returns
     *     the same fallback for every entry. */
    const char *mqtt3_topic = aws_error_str(AWS_ERROR_MQTT_INVALID_TOPIC);
    const char *mqtt5_utf8 = aws_error_str(AWS_ERROR_MQTT5_INVALID_UTF8_STRING);
    assert(strcmp(mqtt3_topic, mqtt5_utf8) != 0);

    aws_mqtt_library_clean_up();
    aws_mqtt_library_init(alloc);
    const char *post_reinit = aws_error_str(AWS_ERROR_MQTT_TIMEOUT);
    assert(is_meaningful(post_reinit));

    aws_mqtt_library_clean_up();
    printf("aws_c_mqtt error-table OK: checked=%zu codes\n", n);
    return 0;
}
