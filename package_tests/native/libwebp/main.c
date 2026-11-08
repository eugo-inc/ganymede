/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libwebp: core API; calls WebPEncodeLosslessRGB(), WebPGetInfo(), WebPDecodeRGB(), WebPFree().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <webp/encode.h>
#include <webp/decode.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t pixels[2 * 2 * 3] = {
        0xff, 0x00, 0x00,  0x00, 0xff, 0x00,
        0x00, 0x00, 0xff,  0xff, 0xff, 0xff,
    };

    uint8_t *encoded = NULL;
    size_t encoded_size = WebPEncodeLosslessRGB(pixels, 2, 2, 2 * 3, &encoded);
    assert(encoded_size > 0);
    assert(encoded != NULL);

    int w = 0, h = 0;
    assert(WebPGetInfo(encoded, encoded_size, &w, &h) == 1);
    assert(w == 2);
    assert(h == 2);

    uint8_t *decoded = WebPDecodeRGB(encoded, encoded_size, &w, &h);
    assert(decoded != NULL);
    assert(w == 2 && h == 2);
    assert(memcmp(decoded, pixels, sizeof(pixels)) == 0);

    WebPFree(decoded);
    WebPFree(encoded);

    printf("libwebp test OK: encoder=0x%06x decoder=0x%06x\n",
           WebPGetEncoderVersion(), WebPGetDecoderVersion());
    return 0;
}
