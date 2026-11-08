/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_cal: hmac; calls aws_default_allocator(), aws_cal_library_init(), aws_byte_cursor_from_array(), aws_sha256_hmac_new().
 */

#include <aws/cal/cal.h>
#include <aws/cal/hmac.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_cal_library_init(alloc);

    static const unsigned char key_bytes[20] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    };
    /* RFC 4231 test case 1 expected HMAC-SHA256. */
    static const unsigned char expected[AWS_SHA256_HMAC_LEN] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
    };

    struct aws_byte_cursor key = aws_byte_cursor_from_array(key_bytes, sizeof(key_bytes));
    struct aws_hmac *hmac = aws_sha256_hmac_new(alloc, &key);
    assert(hmac != NULL);

    struct aws_byte_cursor data = aws_byte_cursor_from_c_str("Hi There");
    assert(aws_hmac_update(hmac, &data) == AWS_OP_SUCCESS);

    unsigned char digest_storage[AWS_SHA256_HMAC_LEN];
    struct aws_byte_buf digest = aws_byte_buf_from_empty_array(digest_storage, AWS_SHA256_HMAC_LEN);
    assert(aws_hmac_finalize(hmac, &digest, 0) == AWS_OP_SUCCESS);
    assert(digest.len == AWS_SHA256_HMAC_LEN);
    assert(memcmp(digest.buffer, expected, AWS_SHA256_HMAC_LEN) == 0);

    aws_hmac_destroy(hmac);
    aws_cal_library_clean_up();

    printf("aws_c_cal HMAC-SHA256 test OK\n");
    return 0;
}
