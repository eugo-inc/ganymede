/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_cal: md5; calls aws_default_allocator(), aws_cal_library_init(), aws_md5_new(), aws_byte_cursor_from_c_str().
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

    static const unsigned char expected[AWS_MD5_LEN] = {
        0x5d, 0x41, 0x40, 0x2a, 0xbc, 0x4b, 0x2a, 0x76,
        0xb9, 0x71, 0x9d, 0x91, 0x10, 0x17, 0xc5, 0x92,
    };

    struct aws_hash *md5 = aws_md5_new(alloc);
    assert(md5 != NULL);

    struct aws_byte_cursor input = aws_byte_cursor_from_c_str("hello");
    assert(aws_hash_update(md5, &input) == AWS_OP_SUCCESS);

    unsigned char digest_storage[AWS_MD5_LEN];
    struct aws_byte_buf digest = aws_byte_buf_from_empty_array(digest_storage, AWS_MD5_LEN);
    assert(aws_hash_finalize(md5, &digest, 0) == AWS_OP_SUCCESS);
    assert(digest.len == AWS_MD5_LEN);
    assert(memcmp(digest.buffer, expected, AWS_MD5_LEN) == 0);

    aws_hash_destroy(md5);
    aws_cal_library_clean_up();

    printf("aws_c_cal MD5 test OK\n");
    return 0;
}
