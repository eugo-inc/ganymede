/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_cal: core API; calls aws_default_allocator(), aws_cal_library_init(), aws_sha256_new(), aws_byte_cursor_from_c_str().
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

    static const unsigned char expected[AWS_SHA256_LEN] = {
        0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
        0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
        0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
        0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
    };

    struct aws_hash *sha256 = aws_sha256_new(alloc);
    assert(sha256 != NULL);

    struct aws_byte_cursor input = aws_byte_cursor_from_c_str("hello");
    assert(aws_hash_update(sha256, &input) == AWS_OP_SUCCESS);

    unsigned char digest_storage[AWS_SHA256_LEN];
    struct aws_byte_buf digest = aws_byte_buf_from_empty_array(digest_storage, AWS_SHA256_LEN);
    assert(aws_hash_finalize(sha256, &digest, 0) == AWS_OP_SUCCESS);
    assert(digest.len == AWS_SHA256_LEN);
    assert(memcmp(digest.buffer, expected, AWS_SHA256_LEN) == 0);

    aws_hash_destroy(sha256);
    aws_cal_library_clean_up();

    printf("aws_c_cal test OK\n");
    return 0;
}
