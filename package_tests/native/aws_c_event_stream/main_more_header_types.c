/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_event_stream: more header types; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_headers_list_init(), aws_event_stream_add_byte_header().
 */

#include <aws/event-stream/event_stream.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_event_stream_library_init(alloc);

    struct aws_array_list headers;
    int rc = aws_event_stream_headers_list_init(&headers, alloc);
    assert(rc == AWS_OP_SUCCESS);

    /* 1. BYTE (int8) header. */
    const char *byte_name = "b";
    const int8_t byte_value = -42;
    rc = aws_event_stream_add_byte_header(&headers, byte_name, (uint8_t)strlen(byte_name), byte_value);
    assert(rc == AWS_OP_SUCCESS);

    /* 2. INT16 header. Both bytes are non-zero, so a wrong byte order fails the read-back. */
    const char *i16_name = "s";
    const int16_t i16_value = (int16_t)0x1234;
    rc = aws_event_stream_add_int16_header(&headers, i16_name, (uint8_t)strlen(i16_name), i16_value);
    assert(rc == AWS_OP_SUCCESS);

    /* 3. INT64 header. Use a value that spans all 8 bytes. */
    const char *i64_name = "l";
    const int64_t i64_value = (int64_t)0x0123456789ABCDEFLL;
    rc = aws_event_stream_add_int64_header(&headers, i64_name, (uint8_t)strlen(i64_name), i64_value);
    assert(rc == AWS_OP_SUCCESS);

    /* 4. BYTE_BUF header with copy=1. Exercises the variable_len_val branch
     * of the header_value union. */
    const char *bb_name = "bb";
    uint8_t bb_value[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
    rc = aws_event_stream_add_bytebuf_header(
        &headers,
        bb_name,
        (uint8_t)strlen(bb_name),
        bb_value,
        (uint16_t)sizeof(bb_value),
        1 /* copy */);
    assert(rc == AWS_OP_SUCCESS);

    /* 5. TIMESTAMP header (int64 millis since epoch). */
    const char *ts_name = "t";
    const int64_t ts_value = 1717000000000LL; /* arbitrary ms-since-epoch */
    rc = aws_event_stream_add_timestamp_header(&headers, ts_name, (uint8_t)strlen(ts_name), ts_value);
    assert(rc == AWS_OP_SUCCESS);

    /* 6. UUID header — exactly 16 bytes, exercises the 16-byte static_val. */
    const char *uuid_name = "u";
    const uint8_t uuid_value[16] = {
        0x55, 0x0e, 0x84, 0x00, 0xe2, 0x9b, 0x41, 0xd4,
        0xa7, 0x16, 0x44, 0x66, 0x55, 0x44, 0x00, 0x00,
    };
    rc = aws_event_stream_add_uuid_header(&headers, uuid_name, (uint8_t)strlen(uuid_name), uuid_value);
    assert(rc == AWS_OP_SUCCESS);

    size_t hdr_count = aws_array_list_length(&headers);
    assert(hdr_count == 6);

    struct aws_event_stream_header_value_pair *h = NULL;

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 0);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_BYTE);
    assert(aws_event_stream_header_value_as_byte(h) == byte_value);

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 1);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_INT16);
    assert(aws_event_stream_header_value_as_int16(h) == i16_value);

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 2);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_INT64);
    assert(aws_event_stream_header_value_as_int64(h) == i64_value);

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 3);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_BYTE_BUF);
    struct aws_byte_buf bb_round = aws_event_stream_header_value_as_bytebuf(h);
    assert(bb_round.len == sizeof(bb_value));
    assert(memcmp(bb_round.buffer, bb_value, sizeof(bb_value)) == 0);

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 4);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_TIMESTAMP);
    assert(aws_event_stream_header_value_as_timestamp(h) == ts_value);

    rc = aws_array_list_get_at_ptr(&headers, (void **)&h, 5);
    assert(rc == AWS_OP_SUCCESS && h != NULL);
    assert(h->header_value_type == AWS_EVENT_STREAM_HEADER_UUID);
    struct aws_byte_buf uuid_round = aws_event_stream_header_value_as_uuid(h);
    assert(uuid_round.len == 16);
    assert(memcmp(uuid_round.buffer, uuid_value, 16) == 0);

    aws_event_stream_headers_list_cleanup(&headers);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream more-header-types test OK: count=%zu i64=0x%016llx\n",
           hdr_count, (unsigned long long)i64_value);
    return 0;
}
