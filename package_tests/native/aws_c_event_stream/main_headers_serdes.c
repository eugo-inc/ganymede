/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_event_stream: headers serdes; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_headers_list_init(), aws_event_stream_add_string_header().
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

    const char *str_name = "x-demo";
    const char *str_value = "hello";
    rc = aws_event_stream_add_string_header(
        &headers,
        str_name,
        (uint8_t)strlen(str_name),
        str_value,
        (uint16_t)strlen(str_value),
        1 /* copy */);
    assert(rc == AWS_OP_SUCCESS);

    const char *i32_name = "n";
    const int32_t i32_value = 0x55AA55AA;
    rc = aws_event_stream_add_int32_header(&headers, i32_name, (uint8_t)strlen(i32_name), i32_value);
    assert(rc == AWS_OP_SUCCESS);

    /* compute_headers_required_buffer_len must return a strictly positive
     * value for a non-empty list. */
    uint32_t required = aws_event_stream_compute_headers_required_buffer_len(&headers);
    assert(required > 0);

    /* Allocate a wire buffer of EXACTLY the required size and serialize
     * into it. Use the _safe variant — the unsafe one is deprecated. */
    struct aws_byte_buf out;
    rc = aws_byte_buf_init(&out, alloc, (size_t)required);
    assert(rc == AWS_OP_SUCCESS);

    rc = aws_event_stream_write_headers_to_buffer_safe(&headers, &out);
    assert(rc == AWS_OP_SUCCESS);

    /* The safe writer must fill the buffer exactly — len == required. */
    assert(out.len == (size_t)required);

    struct aws_array_list parsed_headers;
    rc = aws_event_stream_headers_list_init(&parsed_headers, alloc);
    assert(rc == AWS_OP_SUCCESS);

    rc = aws_event_stream_read_headers_from_buffer(&parsed_headers, out.buffer, out.len);
    assert(rc == AWS_OP_SUCCESS);

    size_t parsed_count = aws_array_list_length(&parsed_headers);
    assert(parsed_count == 2);

    struct aws_event_stream_header_value_pair *p0 = NULL;
    rc = aws_array_list_get_at_ptr(&parsed_headers, (void **)&p0, 0);
    assert(rc == AWS_OP_SUCCESS && p0 != NULL);
    assert(p0->header_value_type == AWS_EVENT_STREAM_HEADER_STRING);
    assert(p0->header_name_len == (uint8_t)strlen(str_name));
    assert(memcmp(p0->header_name, str_name, p0->header_name_len) == 0);
    struct aws_byte_buf p0_val = aws_event_stream_header_value_as_string(p0);
    assert(p0_val.len == strlen(str_value));
    assert(memcmp(p0_val.buffer, str_value, p0_val.len) == 0);

    struct aws_event_stream_header_value_pair *p1 = NULL;
    rc = aws_array_list_get_at_ptr(&parsed_headers, (void **)&p1, 1);
    assert(rc == AWS_OP_SUCCESS && p1 != NULL);
    assert(p1->header_value_type == AWS_EVENT_STREAM_HEADER_INT32);
    assert(p1->header_name_len == (uint8_t)strlen(i32_name));
    assert(memcmp(p1->header_name, i32_name, p1->header_name_len) == 0);
    assert(aws_event_stream_header_value_as_int32(p1) == i32_value);

    aws_event_stream_headers_list_cleanup(&parsed_headers);
    aws_byte_buf_clean_up(&out);
    aws_event_stream_headers_list_cleanup(&headers);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream headers-serdes test OK: required=%u parsed=%zu\n",
           required, parsed_count);
    return 0;
}
