/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_event_stream: headers; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_headers_list_init(), aws_event_stream_add_string_header().
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

    const char *str_name = "content-type";
    const char *str_value = "application/json";
    rc = aws_event_stream_add_string_header(
        &headers,
        str_name,
        (uint8_t)strlen(str_name),
        str_value,
        (uint16_t)strlen(str_value),
        1 /* copy */);
    assert(rc == AWS_OP_SUCCESS);

    const char *i32_name = "event-id";
    const int32_t i32_value = 0x0000A00C;
    rc = aws_event_stream_add_int32_header(&headers, i32_name, (uint8_t)strlen(i32_name), i32_value);
    assert(rc == AWS_OP_SUCCESS);

    const char *bool_name = "is-final";
    rc = aws_event_stream_add_bool_header(&headers, bool_name, (uint8_t)strlen(bool_name), 1 /* true */);
    assert(rc == AWS_OP_SUCCESS);

    /* Three headers should be on the list. */
    size_t hdr_count = aws_array_list_length(&headers);
    assert(hdr_count == 3);

    struct aws_event_stream_header_value_pair *h0 = NULL;
    rc = aws_array_list_get_at_ptr(&headers, (void **)&h0, 0);
    assert(rc == AWS_OP_SUCCESS && h0 != NULL);
    assert(h0->header_value_type == AWS_EVENT_STREAM_HEADER_STRING);
    struct aws_byte_buf v0 = aws_event_stream_header_value_as_string(h0);
    assert(v0.len == strlen(str_value));
    assert(memcmp(v0.buffer, str_value, v0.len) == 0);

    struct aws_event_stream_header_value_pair *h1 = NULL;
    rc = aws_array_list_get_at_ptr(&headers, (void **)&h1, 1);
    assert(rc == AWS_OP_SUCCESS && h1 != NULL);
    assert(h1->header_value_type == AWS_EVENT_STREAM_HEADER_INT32);
    int32_t v1 = aws_event_stream_header_value_as_int32(h1);
    assert(v1 == i32_value);

    struct aws_event_stream_header_value_pair *h2 = NULL;
    rc = aws_array_list_get_at_ptr(&headers, (void **)&h2, 2);
    assert(rc == AWS_OP_SUCCESS && h2 != NULL);
    /* bool headers store true/false in the TYPE itself, not in the value. */
    assert(h2->header_value_type == AWS_EVENT_STREAM_HEADER_BOOL_TRUE);
    int8_t v2 = aws_event_stream_header_value_as_bool(h2);
    assert(v2 == 1);

    aws_event_stream_headers_list_cleanup(&headers);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream headers test OK: count=%zu int32=0x%08x\n",
           hdr_count, (unsigned)v1);
    return 0;
}
