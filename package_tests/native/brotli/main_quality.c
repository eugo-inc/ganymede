/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for brotli: quality; calls BrotliEncoderCreateInstance(), BrotliEncoderSetParameter(), BrotliEncoderCompressStream(), BrotliEncoderIsFinished().
 */

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static size_t compress_at(int quality, int mode,
                          const unsigned char *payload, size_t payload_len,
                          unsigned char *out, size_t out_cap) {
    BrotliEncoderState *enc = BrotliEncoderCreateInstance(NULL, NULL, NULL);
    assert(enc != NULL);

    assert(BrotliEncoderSetParameter(enc, BROTLI_PARAM_QUALITY,
                                     (uint32_t)quality) == BROTLI_TRUE);
    assert(BrotliEncoderSetParameter(enc, BROTLI_PARAM_MODE,
                                     (uint32_t)mode) == BROTLI_TRUE);
    assert(BrotliEncoderSetParameter(enc, BROTLI_PARAM_LGWIN,
                                     BROTLI_DEFAULT_WINDOW) == BROTLI_TRUE);

    size_t available_in = payload_len;
    const uint8_t *next_in = payload;
    size_t available_out = out_cap;
    uint8_t *next_out = out;
    size_t total_out = 0;

    assert(BrotliEncoderCompressStream(
        enc, BROTLI_OPERATION_FINISH,
        &available_in, &next_in,
        &available_out, &next_out,
        &total_out) == BROTLI_TRUE);
    assert(available_in == 0);
    assert(BrotliEncoderIsFinished(enc) == BROTLI_TRUE);

    BrotliEncoderDestroyInstance(enc);
    return out_cap - available_out;
}

int main(void) {
    /* Long, highly-compressible textual payload — max quality should beat
     * min quality (or at minimum match it). */
    static const char text[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
    const size_t text_len = sizeof(text) - 1;

    unsigned char buf_min[2048], buf_max[2048];
    const size_t len_min = compress_at(
        BROTLI_MIN_QUALITY, BROTLI_MODE_TEXT,
        (const unsigned char *)text, text_len, buf_min, sizeof(buf_min));
    const size_t len_max = compress_at(
        BROTLI_MAX_QUALITY, BROTLI_MODE_TEXT,
        (const unsigned char *)text, text_len, buf_max, sizeof(buf_max));

    assert(len_min > 0);
    assert(len_max > 0);
    assert(len_min < text_len);
    assert(len_max < text_len);
    /* Higher quality must not produce a LARGER output. (For highly-repetitive
     * inputs the two endpoints can sometimes tie; the contract is monotonic
     * non-increasing, not strictly smaller.) */
    assert(len_max <= len_min);

    /* Verify the MAX_QUALITY output decompresses cleanly back to the input. */
    unsigned char decoded[2048];
    size_t decoded_len = sizeof(decoded);
    assert(BrotliDecoderDecompress(len_max, buf_max, &decoded_len, decoded)
           == BROTLI_DECODER_RESULT_SUCCESS);
    assert(decoded_len == text_len);
    assert(memcmp(decoded, text, text_len) == 0);

    unsigned char buf_mid[2048];
    const size_t len_mid = compress_at(
        5, BROTLI_MODE_TEXT,
        (const unsigned char *)text, text_len, buf_mid, sizeof(buf_mid));
    assert(len_mid > 0);
    assert(len_mid < text_len);

    unsigned char decoded_mid[2048];
    size_t decoded_mid_len = sizeof(decoded_mid);
    assert(BrotliDecoderDecompress(len_mid, buf_mid, &decoded_mid_len,
                                   decoded_mid)
           == BROTLI_DECODER_RESULT_SUCCESS);
    assert(decoded_mid_len == text_len);
    assert(memcmp(decoded_mid, text, text_len) == 0);

    printf("brotli quality test OK: q=%d->%zu, q=5->%zu, q=%d->%zu bytes\n",
           BROTLI_MIN_QUALITY, len_min, len_mid, BROTLI_MAX_QUALITY, len_max);
    return 0;
}
