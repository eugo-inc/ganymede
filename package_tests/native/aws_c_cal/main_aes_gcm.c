/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_cal: aes gcm; calls aws_default_allocator(), aws_cal_library_init(), aws_byte_cursor_from_array(), aws_aes_gcm_256_new().
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
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4,
    };
    /* AES-GCM IV is conventionally 12 bytes. */
    static const unsigned char iv_bytes[12] = {
        0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
        0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88,
    };
    static const unsigned char aad_bytes[16] = {
        0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
        0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
    };
    static const char plaintext_str[] = "demo aws_c_cal AES-GCM-256 authenticated probe";
    const size_t plaintext_len = sizeof(plaintext_str) - 1;

    struct aws_byte_cursor key = aws_byte_cursor_from_array(key_bytes, sizeof(key_bytes));
    struct aws_byte_cursor iv = aws_byte_cursor_from_array(iv_bytes, sizeof(iv_bytes));
    struct aws_byte_cursor aad = aws_byte_cursor_from_array(aad_bytes, sizeof(aad_bytes));

    /* ---- Encrypt ---- */
    struct aws_symmetric_cipher *enc = aws_aes_gcm_256_new(alloc, &key, &iv, &aad);
    assert(enc != NULL);

    struct aws_byte_cursor plaintext = aws_byte_cursor_from_array(plaintext_str, plaintext_len);
    struct aws_byte_buf ciphertext;
    assert(aws_byte_buf_init(&ciphertext, alloc, plaintext_len + 64) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_encrypt(enc, plaintext, &ciphertext) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_finalize_encryption(enc, &ciphertext) == AWS_OP_SUCCESS);
    assert(ciphertext.len >= plaintext_len);
    assert(memcmp(ciphertext.buffer, plaintext_str, plaintext_len) != 0);

    /* Capture the GMAC tag BEFORE destroying the cipher. */
    struct aws_byte_cursor tag_view = aws_symmetric_cipher_get_tag(enc);
    assert(tag_view.len > 0);

    struct aws_byte_buf tag_copy;
    assert(aws_byte_buf_init_copy_from_cursor(&tag_copy, alloc, tag_view) == AWS_OP_SUCCESS);
    aws_symmetric_cipher_destroy(enc);

    /* ---- Decrypt (positive) ---- */
    struct aws_symmetric_cipher *dec = aws_aes_gcm_256_new(alloc, &key, &iv, &aad);
    assert(dec != NULL);

    struct aws_byte_cursor good_tag = aws_byte_cursor_from_buf(&tag_copy);
    aws_symmetric_cipher_set_tag(dec, good_tag);

    struct aws_byte_cursor ct_cursor = aws_byte_cursor_from_buf(&ciphertext);
    struct aws_byte_buf recovered;
    assert(aws_byte_buf_init(&recovered, alloc, ciphertext.len + 64) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_decrypt(dec, ct_cursor, &recovered) == AWS_OP_SUCCESS);
    assert(aws_symmetric_cipher_finalize_decryption(dec, &recovered) == AWS_OP_SUCCESS);
    assert(recovered.len == plaintext_len);
    assert(memcmp(recovered.buffer, plaintext_str, plaintext_len) == 0);

    aws_symmetric_cipher_destroy(dec);
    aws_byte_buf_clean_up(&recovered);

    /* ---- Decrypt (negative): corrupt the tag — must fail somewhere in the
       decrypt → finalize path. */
    struct aws_byte_buf bad_tag_buf;
    assert(aws_byte_buf_init_copy(&bad_tag_buf, alloc, &tag_copy) == AWS_OP_SUCCESS);
    bad_tag_buf.buffer[0] ^= 0xFFu;

    struct aws_symmetric_cipher *dec_bad = aws_aes_gcm_256_new(alloc, &key, &iv, &aad);
    assert(dec_bad != NULL);
    struct aws_byte_cursor bad_tag = aws_byte_cursor_from_buf(&bad_tag_buf);
    aws_symmetric_cipher_set_tag(dec_bad, bad_tag);

    struct aws_byte_cursor ct_cursor_bad = aws_byte_cursor_from_buf(&ciphertext);
    struct aws_byte_buf recovered_bad;
    assert(aws_byte_buf_init(&recovered_bad, alloc, ciphertext.len + 64) == AWS_OP_SUCCESS);
    int dec_rc = aws_symmetric_cipher_decrypt(dec_bad, ct_cursor_bad, &recovered_bad);
    int fin_rc = aws_symmetric_cipher_finalize_decryption(dec_bad, &recovered_bad);
    /* One of decrypt/finalize must report failure (libcrypto verifies the tag
       at finalize time; the streaming decrypt itself may succeed). */
    assert(dec_rc != AWS_OP_SUCCESS || fin_rc != AWS_OP_SUCCESS);

    aws_symmetric_cipher_destroy(dec_bad);
    aws_byte_buf_clean_up(&recovered_bad);
    aws_byte_buf_clean_up(&bad_tag_buf);
    aws_byte_buf_clean_up(&tag_copy);
    aws_byte_buf_clean_up(&ciphertext);

    aws_cal_library_clean_up();

    printf("aws_c_cal AES-GCM-256 auth-encrypt round-trip + bad-tag rejection test OK\n");
    return 0;
}
