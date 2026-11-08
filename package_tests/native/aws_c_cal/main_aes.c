/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_cal: aes; calls aws_default_allocator(), aws_cal_library_init(), aws_byte_cursor_from_array(), aws_aes_ctr_256_new().
 */

#include <aws/cal/cal.h>
#include <aws/cal/symmetric_cipher.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_cal_library_init(alloc);

    static const unsigned char key_bytes[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    };
    static const unsigned char iv_bytes[16] = {
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };
    static const char plaintext_str[] = "demo aws_c_cal AES-CTR round-trip probe!!";
    const size_t plaintext_len = sizeof(plaintext_str) - 1;

    struct aws_byte_cursor key = aws_byte_cursor_from_array(key_bytes, sizeof(key_bytes));
    struct aws_byte_cursor iv = aws_byte_cursor_from_array(iv_bytes, sizeof(iv_bytes));

    /* ---- Encrypt ---- */
    struct aws_symmetric_cipher *enc = aws_aes_ctr_256_new(alloc, &key, &iv);
    assert(enc != NULL);

    struct aws_byte_cursor plaintext = aws_byte_cursor_from_array(plaintext_str, plaintext_len);
    struct aws_byte_buf ciphertext;
    assert(aws_byte_buf_init(&ciphertext, alloc, plaintext_len + 64) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_encrypt(enc, plaintext, &ciphertext) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_finalize_encryption(enc, &ciphertext) == AWS_OP_SUCCESS);

    /* Ciphertext must be at least as long as plaintext and must DIFFER from it. */
    assert(ciphertext.len >= plaintext_len);
    assert(memcmp(ciphertext.buffer, plaintext_str, plaintext_len) != 0);

    aws_symmetric_cipher_destroy(enc);

    /* ---- Decrypt ---- */
    struct aws_symmetric_cipher *dec = aws_aes_ctr_256_new(alloc, &key, &iv);
    assert(dec != NULL);

    struct aws_byte_cursor ct_cursor = aws_byte_cursor_from_buf(&ciphertext);
    struct aws_byte_buf recovered;
    assert(aws_byte_buf_init(&recovered, alloc, ciphertext.len + 64) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_decrypt(dec, ct_cursor, &recovered) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_finalize_decryption(dec, &recovered) == AWS_OP_SUCCESS);

    assert(recovered.len == plaintext_len);
    assert(memcmp(recovered.buffer, plaintext_str, plaintext_len) == 0);

    aws_symmetric_cipher_destroy(dec);
    aws_byte_buf_clean_up(&ciphertext);
    aws_byte_buf_clean_up(&recovered);

    aws_cal_library_clean_up();

    printf("aws_c_cal AES-CTR-256 round-trip test OK\n");
    return 0;
}
