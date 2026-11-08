/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nettle: base64; calls base64_encode_init(), base64_encode_update(), base64_encode_final(), base64_decode_init().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nettle/base64.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char plaintext[] = "foobar";
    static const size_t pt_len = sizeof(plaintext) - 1;
    static const char expected[] = "Zm9vYmFy";
    static const size_t expected_len = sizeof(expected) - 1;

    /* --- Encode --- */
    struct base64_encode_ctx enc;
    /* BASE64_ENCODE_LENGTH excludes any final padding/newline byte. */
    char encoded[BASE64_ENCODE_LENGTH(pt_len) + BASE64_ENCODE_FINAL_LENGTH + 1];
    size_t written = 0;
    base64_encode_init(&enc);
    written += base64_encode_update(&enc, (uint8_t *)encoded, pt_len,
                                    (const uint8_t *)plaintext);
    written += base64_encode_final(&enc, (uint8_t *)encoded + written);
    encoded[written] = '\0';
    assert(written == expected_len);
    assert(memcmp(encoded, expected, expected_len) == 0);

    struct base64_decode_ctx dec;
    uint8_t decoded[BASE64_DECODE_LENGTH(expected_len) + 1];
    /* nettle: *dst_length is IN/OUT — seed it with the buffer capacity or the
     * decoder refuses to write (it errors if output would exceed *dst_length). */
    size_t decoded_len = sizeof(decoded);
    base64_decode_init(&dec);
    int ok = base64_decode_update(&dec, &decoded_len, decoded,
                                  expected_len, (const uint8_t *)expected);
    assert(ok == 1);
    ok = base64_decode_final(&dec);
    assert(ok == 1);
    assert(decoded_len == pt_len);
    assert(memcmp(decoded, plaintext, pt_len) == 0);

    struct base64_decode_ctx bad;
    uint8_t bad_out[8];
    /* IN/OUT capacity seeded so the rejection is genuinely the garbage input,
     * not a zero-size output buffer. */
    size_t bad_len = sizeof(bad_out);
    base64_decode_init(&bad);
    int bad_ok = base64_decode_update(&bad, &bad_len, bad_out,
                                      4, (const uint8_t *)"!!!!");
    assert(bad_ok == 0);

    printf("nettle base64 test OK\n");
    return 0;
}
