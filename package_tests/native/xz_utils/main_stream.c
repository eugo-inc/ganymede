/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xz_utils: stream; calls lzma_easy_encoder(), lzma_code(), lzma_end(), lzma_stream_decoder().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    static const unsigned char input[] =
        "alpha bravo charlie delta echo foxtrot golf hotel india juliet "
        "alpha bravo charlie delta echo foxtrot golf hotel india juliet "
        "alpha bravo charlie delta echo foxtrot golf hotel india juliet "
        "alpha bravo charlie delta echo foxtrot golf hotel india juliet";
    const size_t input_size = sizeof(input) - 1;

    /* ---------- ENCODE ---------- */
    lzma_stream enc = LZMA_STREAM_INIT;
    lzma_ret rc = lzma_easy_encoder(&enc, 6, LZMA_CHECK_CRC64);
    assert(rc == LZMA_OK);

    unsigned char compressed[1024];
    enc.next_in = input;
    enc.avail_in = input_size;
    enc.next_out = compressed;
    enc.avail_out = sizeof(compressed);

    /* Run the encoder to consume all input. */
    rc = lzma_code(&enc, LZMA_RUN);
    assert(rc == LZMA_OK);
    assert(enc.avail_in == 0);

    /* Finalize. */
    rc = lzma_code(&enc, LZMA_FINISH);
    assert(rc == LZMA_STREAM_END);

    const size_t compressed_size = sizeof(compressed) - enc.avail_out;
    assert(compressed_size > 0);
    assert(compressed_size < input_size);  /* sanity: there IS some compression */
    lzma_end(&enc);

    /* ---------- DECODE ---------- */
    lzma_stream dec = LZMA_STREAM_INIT;
    rc = lzma_stream_decoder(&dec, UINT64_MAX, 0);
    assert(rc == LZMA_OK);

    unsigned char decompressed[sizeof(input)];
    dec.next_in = compressed;
    dec.avail_in = compressed_size;
    dec.next_out = decompressed;
    dec.avail_out = sizeof(decompressed);

    rc = lzma_code(&dec, LZMA_FINISH);
    assert(rc == LZMA_STREAM_END);

    const size_t decompressed_size = sizeof(decompressed) - dec.avail_out;
    assert(decompressed_size == input_size);
    assert(memcmp(decompressed, input, input_size) == 0);
    lzma_end(&dec);

    printf("xz_utils stream test OK: compressed %zu -> %zu bytes\n",
           input_size, compressed_size);
    return 0;
}
