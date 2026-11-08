/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lz4: frame; calls LZ4F_compressFrameBound(), LZ4F_compressFrame(), LZ4F_isError(), LZ4F_createDecompressionContext().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lz4frame.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *input =
        "lz4 frame format round-trip test: "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const size_t input_len = strlen(input);

    /* --- Compress into a self-contained frame. --- */
    const size_t bound = LZ4F_compressFrameBound(input_len, NULL);
    assert(bound > 0);

    char *frame = (char *)malloc(bound);
    assert(frame != NULL);

    const size_t frame_len = LZ4F_compressFrame(frame, bound,
                                                input, input_len,
                                                NULL /* prefs = defaults */);
    assert(!LZ4F_isError(frame_len));
    assert(frame_len > 4);

    /* LZ4 frame magic number (little-endian 0x184D2204). */
    assert((unsigned char)frame[0] == 0x04);
    assert((unsigned char)frame[1] == 0x22);
    assert((unsigned char)frame[2] == 0x4D);
    assert((unsigned char)frame[3] == 0x18);

    /* --- Decompress through a streaming context. --- */
    LZ4F_dctx *dctx = NULL;
    const LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    assert(!LZ4F_isError(err));
    assert(dctx != NULL);

    char output[1024] = {0};
    size_t out_size = sizeof(output);
    size_t in_size = frame_len;
    const size_t hint = LZ4F_decompress(dctx, output, &out_size,
                                        frame, &in_size, NULL);
    assert(!LZ4F_isError(hint));
    /* hint == 0 means the frame was fully consumed. */
    assert(hint == 0);
    assert(in_size == frame_len);
    assert(out_size == input_len);
    assert(memcmp(output, input, input_len) == 0);

    LZ4F_freeDecompressionContext(dctx);
    free(frame);

    printf("lz4 frame test OK: input=%zu frame=%zu\n", input_len, frame_len);
    return 0;
}
