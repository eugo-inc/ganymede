/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_event_stream: core API; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_message_init(), aws_event_stream_message_total_length().
 */

#include <aws/event-stream/event_stream.h>
#include <aws/common/allocator.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_event_stream_library_init(alloc);

    struct aws_event_stream_message msg;
    int rc = aws_event_stream_message_init(&msg, alloc, NULL, NULL);
    assert(rc == AWS_OP_SUCCESS);

    uint32_t total = aws_event_stream_message_total_length(&msg);
    assert(total == 16);

    uint32_t headers_len = aws_event_stream_message_headers_len(&msg);
    assert(headers_len == 0);

    uint32_t payload_len = aws_event_stream_message_payload_len(&msg);
    assert(payload_len == 0);

    aws_event_stream_message_clean_up(&msg);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream test OK: empty_frame_len=%u\n", total);
    return 0;
}
