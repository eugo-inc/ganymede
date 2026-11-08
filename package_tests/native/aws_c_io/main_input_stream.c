/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_io: input stream; calls aws_default_allocator(), aws_io_library_init(), aws_input_stream_new_from_cursor(), aws_input_stream_get_length().
 */

#include <aws/io/io.h>
#include <aws/io/stream.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    const char payload[] = "demo-aws-c-io-input-stream-probe";
    const size_t payload_len = sizeof(payload) - 1;
    struct aws_byte_cursor cursor = {
        .ptr = (uint8_t *)payload,
        .len = payload_len,
    };

    struct aws_input_stream *stream = aws_input_stream_new_from_cursor(alloc, &cursor);
    assert(stream != NULL);

    /* Reported length must match the payload — catches off-by-one or
     * cursor-len-truncation bugs in the cursor stream impl. */
    int64_t reported_len = -1;
    int rc = aws_input_stream_get_length(stream, &reported_len);
    assert(rc == AWS_OP_SUCCESS);
    assert((size_t)reported_len == payload_len);

    /* Read into a buffer one byte larger than needed, then assert the
     * stream reports end-of-stream and the bytes match. */
    struct aws_byte_buf dest;
    rc = aws_byte_buf_init(&dest, alloc, payload_len + 1);
    assert(rc == AWS_OP_SUCCESS);

    rc = aws_input_stream_read(stream, &dest);
    assert(rc == AWS_OP_SUCCESS);
    assert(dest.len == payload_len);
    assert(memcmp(dest.buffer, payload, payload_len) == 0);

    struct aws_stream_status status = {0};
    rc = aws_input_stream_get_status(stream, &status);
    assert(rc == AWS_OP_SUCCESS);
    assert(status.is_valid);
    assert(status.is_end_of_stream);

    aws_byte_buf_clean_up(&dest);
    aws_input_stream_release(stream);

    aws_io_library_clean_up();

    printf("aws_c_io input_stream test OK: len=%lld\n", (long long)reported_len);
    return 0;
}
