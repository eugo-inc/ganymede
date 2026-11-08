/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for brotli: stream; calls BrotliEncoderCreateInstance(), BrotliEncoderIsFinished(), BrotliEncoderCompressStream(), BrotliEncoderDestroyInstance().
 */

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const char payload[] =
        "Demo HPC streaming compression round-trip test payload. "
        "Demo HPC streaming compression round-trip test payload. "
        "Demo HPC streaming compression round-trip test payload. "
        "Demo HPC streaming compression round-trip test payload.";
    const size_t payload_len = sizeof(payload) - 1;

    /* ---- streaming encode ---- */
    BrotliEncoderState *enc = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    assert(enc != NULL);

    unsigned char encoded[1024];
    size_t available_in = payload_len;
    const uint8_t *next_in = (const uint8_t *)payload;
    size_t available_out = sizeof(encoded);
    uint8_t *next_out = encoded;
    size_t total_out = 0;

    assert(BrotliEncoderIsFinished(enc) == BROTLI_FALSE);

    assert(BrotliEncoderCompressStream(
        enc, BROTLI_OPERATION_FINISH,
        &available_in, &next_in,
        &available_out, &next_out,
        &total_out) == BROTLI_TRUE);

    assert(available_in == 0);  /* all input consumed in one call. */
    assert(BrotliEncoderIsFinished(enc) == BROTLI_TRUE);

    const size_t encoded_len = sizeof(encoded) - available_out;
    assert(encoded_len > 0);
    assert(encoded_len == total_out);
    assert(encoded_len < payload_len);  /* repeated phrase must compress. */

    BrotliEncoderDestroyInstance(enc);

    /* ---- streaming decode ---- */
    BrotliDecoderState *dec = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    assert(dec != NULL);

    unsigned char decoded[1024];
    size_t d_available_in = encoded_len;
    const uint8_t *d_next_in = encoded;
    size_t d_available_out = sizeof(decoded);
    uint8_t *d_next_out = decoded;
    size_t d_total_out = 0;

    BrotliDecoderResult result = BrotliDecoderDecompressStream(
        dec, &d_available_in, &d_next_in,
        &d_available_out, &d_next_out, &d_total_out);
    assert(result == BROTLI_DECODER_RESULT_SUCCESS);
    assert(BrotliDecoderIsFinished(dec) == BROTLI_TRUE);
    assert(d_total_out == payload_len);
    assert(memcmp(decoded, payload, payload_len) == 0);

    BrotliDecoderDestroyInstance(dec);

    printf("brotli stream test OK: %zu -> %zu -> %zu bytes\n",
           payload_len, encoded_len, d_total_out);
    return 0;
}
