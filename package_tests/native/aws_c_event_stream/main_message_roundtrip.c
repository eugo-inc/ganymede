/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_event_stream: message roundtrip; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_headers_list_init(), aws_event_stream_add_string_header().
 */

#include <aws/event-stream/event_stream.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_event_stream_library_init(alloc);

    struct aws_array_list headers;
    int rc = aws_event_stream_headers_list_init(&headers, alloc);
    assert(rc == AWS_OP_SUCCESS);

    const char *hdr_name = "content-type";
    const char *hdr_value = "application/json";
    rc = aws_event_stream_add_string_header(
        &headers,
        hdr_name,
        (uint8_t)strlen(hdr_name),
        hdr_value,
        (uint16_t)strlen(hdr_value),
        0);
    assert(rc == AWS_OP_SUCCESS);

    const char *payload_str = "{\"hello\":\"demo\"}";
    struct aws_byte_buf payload = aws_byte_buf_from_c_str(payload_str);

    struct aws_event_stream_message msg;
    rc = aws_event_stream_message_init(&msg, alloc, &headers, &payload);
    assert(rc == AWS_OP_SUCCESS);

    uint32_t orig_total = aws_event_stream_message_total_length(&msg);
    uint32_t orig_headers_len = aws_event_stream_message_headers_len(&msg);
    uint32_t orig_payload_len = aws_event_stream_message_payload_len(&msg);
    uint32_t orig_msg_crc = aws_event_stream_message_message_crc(&msg);
    assert(orig_total >= 16);
    assert(orig_headers_len > 0);
    assert(orig_payload_len == strlen(payload_str));

    const uint8_t *wire = aws_event_stream_message_buffer(&msg);
    assert(wire != NULL);
    uint8_t *wire_copy = (uint8_t *)malloc(orig_total);
    assert(wire_copy != NULL);
    memcpy(wire_copy, wire, orig_total);

    aws_event_stream_message_clean_up(&msg);
    aws_event_stream_headers_list_cleanup(&headers);

    /* Re-parse: any CRC mismatch or framing drift would fail here. */
    struct aws_byte_buf parsed_buf = aws_byte_buf_from_array(wire_copy, orig_total);
    struct aws_event_stream_message parsed;
    rc = aws_event_stream_message_from_buffer_copy(&parsed, alloc, &parsed_buf);
    assert(rc == AWS_OP_SUCCESS);

    assert(aws_event_stream_message_total_length(&parsed) == orig_total);
    assert(aws_event_stream_message_headers_len(&parsed) == orig_headers_len);
    assert(aws_event_stream_message_payload_len(&parsed) == orig_payload_len);
    assert(aws_event_stream_message_message_crc(&parsed) == orig_msg_crc);

    const uint8_t *parsed_payload = aws_event_stream_message_payload(&parsed);
    assert(parsed_payload != NULL);
    assert(memcmp(parsed_payload, payload_str, orig_payload_len) == 0);

    aws_event_stream_message_clean_up(&parsed);
    free(wire_copy);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream roundtrip test OK: total=%u hdrs=%u payload=%u crc=0x%08x\n",
           orig_total, orig_headers_len, orig_payload_len, orig_msg_crc);
    return 0;
}
