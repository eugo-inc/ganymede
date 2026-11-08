/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libaec: core API; calls aec_buffer_encode(), aec_buffer_decode().
 *
 * Every library call and comparison runs outside assert(): each result is kept in a
 * variable and checked with if/return, so compiling with -DNDEBUG cannot remove the
 * calls themselves.
 */

#include <libaec.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Twelve 8-bit samples — small enough to compress fast, large enough to exercise the codec. */
    unsigned char input[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    unsigned char encoded[256] = {0};
    unsigned char decoded[12] = {0};

    struct aec_stream enc;
    memset(&enc, 0, sizeof(enc));
    enc.bits_per_sample = 8;
    enc.block_size = 8;
    enc.rsi = 2;
    enc.flags = AEC_DATA_PREPROCESS;
    enc.next_in = input;
    enc.avail_in = sizeof(input);
    enc.next_out = encoded;
    enc.avail_out = sizeof(encoded);
    const int rc_encode = aec_buffer_encode(&enc);
    if (rc_encode != AEC_OK) {
        fprintf(stderr, "libaec FAILED: aec_buffer_encode returned %d, want AEC_OK\n", rc_encode);
        return 1;
    }
    if (enc.total_out == 0) {
        fprintf(stderr, "libaec FAILED: aec_buffer_encode wrote 0 bytes\n");
        return 1;
    }

    struct aec_stream dec;
    memset(&dec, 0, sizeof(dec));
    dec.bits_per_sample = 8;
    dec.block_size = 8;
    dec.rsi = 2;
    dec.flags = AEC_DATA_PREPROCESS;
    dec.next_in = encoded;
    dec.avail_in = enc.total_out;
    dec.next_out = decoded;
    dec.avail_out = sizeof(decoded);
    const int rc_decode = aec_buffer_decode(&dec);
    if (rc_decode != AEC_OK) {
        fprintf(stderr, "libaec FAILED: aec_buffer_decode returned %d, want AEC_OK\n", rc_decode);
        return 1;
    }

    if (memcmp(input, decoded, sizeof(input)) != 0) {
        fprintf(stderr, "libaec FAILED: the encode/decode round trip changed the data\n");
        return 1;
    }

    printf("libaec test OK\n");
    return 0;
}
