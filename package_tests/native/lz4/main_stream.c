/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lz4: stream; calls LZ4_createStream(), LZ4_compressBound(), LZ4_compress_fast_continue(), LZ4_freeStream().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lz4.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define BLOCKS 4
#define BLOCK_BYTES 1024

int main(void) {
    char input[BLOCKS][BLOCK_BYTES];
    for (int b = 0; b < BLOCKS; ++b) {
        for (int i = 0; i < BLOCK_BYTES; ++i) {
            input[b][i] = (char)('A' + b + (i % 16));
        }
    }

    /* --- Streaming compression. --- */
    LZ4_stream_t *cstream = LZ4_createStream();
    assert(cstream != NULL);

    /* compressBound applied per block; oversize destination is safe. */
    const int cap = LZ4_compressBound(BLOCK_BYTES);
    assert(cap > 0);

    char compressed[BLOCKS][2048];
    int compressed_len[BLOCKS];
    assert(cap <= (int)sizeof(compressed[0]));

    for (int b = 0; b < BLOCKS; ++b) {
        compressed_len[b] = LZ4_compress_fast_continue(cstream,
                                                       input[b],
                                                       compressed[b],
                                                       BLOCK_BYTES,
                                                       (int)sizeof(compressed[b]),
                                                       1 /* acceleration */);
        assert(compressed_len[b] > 0);
    }

    LZ4_freeStream(cstream);

    /* --- Streaming decompression — must reuse a single decode context so the
     *     cross-block back-references resolve. --- */
    LZ4_streamDecode_t *dstream = LZ4_createStreamDecode();
    assert(dstream != NULL);
    const int init_ok = LZ4_setStreamDecode(dstream, NULL, 0);
    assert(init_ok == 1);

    char output[BLOCKS][BLOCK_BYTES];
    for (int b = 0; b < BLOCKS; ++b) {
        const int n = LZ4_decompress_safe_continue(dstream,
                                                   compressed[b],
                                                   output[b],
                                                   compressed_len[b],
                                                   BLOCK_BYTES);
        assert(n == BLOCK_BYTES);
        assert(memcmp(output[b], input[b], BLOCK_BYTES) == 0);
    }

    LZ4_freeStreamDecode(dstream);

    printf("lz4 stream test OK: %d blocks x %d bytes round-tripped\n",
           BLOCKS, BLOCK_BYTES);
    return 0;
}
