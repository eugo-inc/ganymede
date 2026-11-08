/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_cal: ecdsa; calls aws_default_allocator(), aws_cal_library_init(), aws_ecc_key_pair_new_generate_random(), aws_byte_cursor_from_array().
 */

#include <aws/cal/cal.h>
#include <aws/cal/ecc.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_cal_library_init(alloc);

    struct aws_ecc_key_pair *key = aws_ecc_key_pair_new_generate_random(alloc, AWS_CAL_ECDSA_P256);
    assert(key != NULL);

    /* Pretend message is already a SHA-256 digest (32 bytes — matches P-256
       coordinate size, which is what ECDSA-P256 expects). */
    static const unsigned char digest_bytes[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    struct aws_byte_cursor message = aws_byte_cursor_from_array(digest_bytes, sizeof(digest_bytes));

    size_t sig_max = aws_ecc_key_pair_signature_length(key);
    assert(sig_max > 0);

    struct aws_byte_buf signature;
    assert(aws_byte_buf_init(&signature, alloc, sig_max) == AWS_OP_SUCCESS);
    assert(aws_ecc_key_pair_sign_message(key, &message, &signature) == AWS_OP_SUCCESS);
    assert(signature.len > 0);
    assert(signature.len <= sig_max);

    struct aws_byte_cursor sig_cursor = aws_byte_cursor_from_buf(&signature);
    assert(aws_ecc_key_pair_verify_signature(key, &message, &sig_cursor) == AWS_OP_SUCCESS);

    assert(signature.len >= 8);
    signature.buffer[signature.len / 2] ^= 0xFFu;
    struct aws_byte_cursor bad_sig_cursor = aws_byte_cursor_from_buf(&signature);
    assert(aws_ecc_key_pair_verify_signature(key, &message, &bad_sig_cursor) != AWS_OP_SUCCESS);

    aws_byte_buf_clean_up(&signature);
    aws_ecc_key_pair_release(key);

    aws_cal_library_clean_up();

    printf("aws_c_cal ECDSA-P256 sign+verify round-trip test OK\n");
    return 0;
}
