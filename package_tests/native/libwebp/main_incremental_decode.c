/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libwebp: incremental decode; calls WebPEncodeLosslessRGB(), WebPINewRGB(), WebPIAppend(), WebPIDecGetRGB().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <webp/encode.h>
#include <webp/decode.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    enum { W = 4, H = 4 };
    static const uint8_t pixels[W * H * 3] = {
        0xff, 0x00, 0x00,  0x00, 0xff, 0x00,  0x00, 0x00, 0xff,  0xff, 0xff, 0xff,
        0x10, 0x20, 0x30,  0x40, 0x50, 0x60,  0x70, 0x80, 0x90,  0xa0, 0xb0, 0xc0,
        0x11, 0x22, 0x33,  0x44, 0x55, 0x66,  0x77, 0x88, 0x99,  0xaa, 0xbb, 0xcc,
        0x01, 0x02, 0x03,  0x04, 0x05, 0x06,  0x07, 0x08, 0x09,  0x0a, 0x0b, 0x0c,
    };

    uint8_t *encoded = NULL;
    size_t encoded_size = WebPEncodeLosslessRGB(pixels, W, H, W * 3, &encoded);
    assert(encoded_size > 0);
    assert(encoded != NULL);
    /* For the split to actually exercise the state machine, the bitstream
     * must be larger than a couple of bytes. */
    assert(encoded_size > 16);

    /* Caller-owned output buffer — incremental API writes into this. */
    uint8_t output[W * H * 3] = {0};
    WebPIDecoder *idec = WebPINewRGB(MODE_RGB, output, sizeof(output), W * 3);
    assert(idec != NULL);

    /* Split the bitstream roughly in half and feed it in two chunks.
     * First chunk: must return VP8_STATUS_SUSPENDED (more data needed).
     * Second chunk: must return VP8_STATUS_OK (decode complete). */
    size_t chunk1 = encoded_size / 2;
    size_t chunk2 = encoded_size - chunk1;

    VP8StatusCode s1 = WebPIAppend(idec, encoded, chunk1);
    assert(s1 == VP8_STATUS_SUSPENDED);

    VP8StatusCode s2 = WebPIAppend(idec, encoded + chunk1, chunk2);
    assert(s2 == VP8_STATUS_OK);

    /* Query the finished output buffer via WebPIDecGetRGB. */
    int last_y = 0, width = 0, height = 0, stride = 0;
    uint8_t *buf = WebPIDecGetRGB(idec, &last_y, &width, &height, &stride);
    assert(buf != NULL);
    assert(width == W);
    assert(height == H);
    assert(stride == W * 3);
    assert(last_y == H);  /* all rows decoded */

    assert(memcmp(buf, pixels, sizeof(pixels)) == 0);

    WebPIDelete(idec);
    WebPFree(encoded);

    printf("libwebp incremental-decode test OK\n");
    return 0;
}
