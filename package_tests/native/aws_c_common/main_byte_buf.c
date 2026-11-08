/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_common: byte buf; calls aws_default_allocator(), aws_common_library_init(), aws_byte_buf_init(), aws_byte_cursor_from_c_str().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    /* Dynamic buffer init — capacity 16, len starts at 0. */
    struct aws_byte_buf buf;
    int rc = aws_byte_buf_init(&buf, alloc, 16);
    assert(rc == AWS_OP_SUCCESS);
    assert(buf.len == 0);
    assert(buf.capacity == 16);
    assert(buf.buffer != NULL);

    /* Append from a c-str cursor. */
    struct aws_byte_cursor c1 = aws_byte_cursor_from_c_str("hello ");
    rc = aws_byte_buf_append(&buf, &c1);
    assert(rc == AWS_OP_SUCCESS);
    assert(buf.len == 6);

    struct aws_byte_cursor c2 = aws_byte_cursor_from_c_str("demo");
    rc = aws_byte_buf_append(&buf, &c2);
    assert(rc == AWS_OP_SUCCESS);
    assert(buf.len == 10);
    assert(memcmp(buf.buffer, "hello demo", 10) == 0);

    struct aws_byte_cursor whole = aws_byte_cursor_from_buf(&buf);
    assert(whole.len == 10);
    struct aws_byte_cursor advanced = aws_byte_cursor_advance(&whole, 6);
    assert(advanced.len == 6);
    assert(memcmp(advanced.ptr, "hello ", 6) == 0);
    assert(whole.len == 4);
    assert(memcmp(whole.ptr, "demo", 4) == 0);

    aws_byte_buf_clean_up(&buf);
    assert(buf.buffer == NULL);
    assert(buf.len == 0);
    assert(buf.capacity == 0);

    aws_common_library_clean_up();
    printf("aws_c_common byte_buf test OK\n");
    return 0;
}
