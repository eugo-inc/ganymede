/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for brotli: core API; calls BrotliEncoderCompress(), BrotliDecoderDecompress(), BrotliEncoderVersion(), BrotliDecoderVersion().
 */

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char payload[] =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog.";
    const size_t payload_len = sizeof(payload) - 1;

    unsigned char encoded[512];
    size_t encoded_len = sizeof(encoded);
    assert(BrotliEncoderCompress(
        BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC,
        payload_len, (const unsigned char *)payload,
        &encoded_len, encoded) == BROTLI_TRUE);
    assert(encoded_len > 0);
    assert(encoded_len < payload_len);  /* the repeated phrase must compress. */

    unsigned char decoded[512];
    size_t decoded_len = sizeof(decoded);
    assert(BrotliDecoderDecompress(encoded_len, encoded, &decoded_len, decoded)
           == BROTLI_DECODER_RESULT_SUCCESS);
    assert(decoded_len == payload_len);
    assert(memcmp(decoded, payload, payload_len) == 0);

    printf("brotli test OK: encoder=%u decoder=%u\n",
           BrotliEncoderVersion(), BrotliDecoderVersion());
    return 0;
}
