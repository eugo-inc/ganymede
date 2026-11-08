/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_mqtt: mqtt5 packet type; calls aws_default_allocator(), aws_mqtt_library_init(), aws_mqtt_library_clean_up().
 */

#include <aws/mqtt/mqtt.h>
#include <aws/mqtt/v5/mqtt5_types.h>
#include <aws/common/allocator.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_mqtt_library_init(alloc);

    assert((int)AWS_MQTT5_PT_CONNECT == 1);
    assert((int)AWS_MQTT5_PT_CONNACK == 2);
    assert((int)AWS_MQTT5_PT_PUBLISH == 3);
    assert((int)AWS_MQTT5_PT_PUBACK == 4);
    assert((int)AWS_MQTT5_PT_PUBREC == 5);
    assert((int)AWS_MQTT5_PT_PUBREL == 6);
    assert((int)AWS_MQTT5_PT_PUBCOMP == 7);
    assert((int)AWS_MQTT5_PT_SUBSCRIBE == 8);
    assert((int)AWS_MQTT5_PT_SUBACK == 9);
    assert((int)AWS_MQTT5_PT_UNSUBSCRIBE == 10);
    assert((int)AWS_MQTT5_PT_UNSUBACK == 11);
    assert((int)AWS_MQTT5_PT_PINGREQ == 12);
    assert((int)AWS_MQTT5_PT_PINGRESP == 13);
    assert((int)AWS_MQTT5_PT_DISCONNECT == 14);
    assert((int)AWS_MQTT5_PT_AUTH == 15);

    assert((int)AWS_MQTT5_PT_CONNECT != (int)AWS_MQTT5_PT_PUBLISH);
    assert((int)AWS_MQTT5_PT_CONNECT != (int)AWS_MQTT5_PT_SUBSCRIBE);
    assert((int)AWS_MQTT5_PT_PUBLISH != (int)AWS_MQTT5_PT_SUBSCRIBE);

    aws_mqtt_library_clean_up();
    printf("aws_c_mqtt mqtt5-packet-type OK: CONNECT=%d PUBLISH=%d SUBSCRIBE=%d AUTH=%d\n",
           (int)AWS_MQTT5_PT_CONNECT,
           (int)AWS_MQTT5_PT_PUBLISH,
           (int)AWS_MQTT5_PT_SUBSCRIBE,
           (int)AWS_MQTT5_PT_AUTH);
    return 0;
}
