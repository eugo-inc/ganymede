/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_cal: sha1; calls aws_default_allocator(), aws_cal_library_init(), aws_byte_cursor_from_c_str(), aws_byte_buf_from_empty_array().
 */

#include <aws/cal/cal.h>
#include <aws/cal/hash.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_cal_library_init(alloc);

    static const unsigned char expected[AWS_SHA1_LEN] = {
        0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e,
        0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d,
    };

    struct aws_byte_cursor input = aws_byte_cursor_from_c_str("abc");

    unsigned char digest_storage[AWS_SHA1_LEN];
    struct aws_byte_buf digest = aws_byte_buf_from_empty_array(digest_storage, AWS_SHA1_LEN);

    /* One-shot compute — distinct internal code path from streaming. */
    assert(aws_sha1_compute(alloc, &input, &digest, 0) == AWS_OP_SUCCESS);
    assert(digest.len == AWS_SHA1_LEN);
    assert(memcmp(digest.buffer, expected, AWS_SHA1_LEN) == 0);

    unsigned char truncated_storage[10];
    struct aws_byte_buf truncated = aws_byte_buf_from_empty_array(truncated_storage, 10);
    assert(aws_sha1_compute(alloc, &input, &truncated, 10) == AWS_OP_SUCCESS);
    assert(truncated.len == 10);
    assert(memcmp(truncated.buffer, expected, 10) == 0);

    aws_cal_library_clean_up();

    printf("aws_c_cal SHA-1 one-shot+truncate test OK\n");
    return 0;
}
