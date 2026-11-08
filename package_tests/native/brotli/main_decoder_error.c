/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for brotli: decoder error; calls BrotliDecoderDecompress(), BrotliDecoderCreateInstance(), BrotliDecoderDecompressStream(), BrotliDecoderIsFinished().
 */

#include <brotli/decode.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Deliberately corrupt bytes that are NOT a valid brotli stream.
     * 0xFF repeated trips the meta-block header parser immediately. */
    static const unsigned char garbage[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    };

    /* ---- one-shot decoder rejects corruption ---- */
    unsigned char out_buf[256];
    size_t out_len = sizeof(out_buf);
    BrotliDecoderResult one_shot = BrotliDecoderDecompress(
        sizeof(garbage), garbage, &out_len, out_buf);
    assert(one_shot == BROTLI_DECODER_RESULT_ERROR);

    /* ---- streaming decoder rejects corruption + reports an error code ---- */
    BrotliDecoderState *dec = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    assert(dec != NULL);

    size_t available_in = sizeof(garbage);
    const uint8_t *next_in = garbage;
    size_t available_out = sizeof(out_buf);
    uint8_t *next_out = out_buf;
    size_t total_out = 0;

    BrotliDecoderResult stream_result = BrotliDecoderDecompressStream(
        dec, &available_in, &next_in,
        &available_out, &next_out, &total_out);
    assert(stream_result == BROTLI_DECODER_RESULT_ERROR);
    assert(BrotliDecoderIsFinished(dec) == BROTLI_FALSE);

    BrotliDecoderErrorCode err = BrotliDecoderGetErrorCode(dec);
    /* The brotli error-code enum convention: success/needs-more are >= 0,
     * every actual error is strictly negative. */
    assert((int)err < 0);

    const char *msg = BrotliDecoderErrorString(err);
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    BrotliDecoderDestroyInstance(dec);

    printf("brotli decoder error test OK: code=%d msg=\"%s\"\n",
           (int)err, msg);
    return 0;
}
