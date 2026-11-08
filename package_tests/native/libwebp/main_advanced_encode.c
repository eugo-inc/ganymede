/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libwebp: advanced encode; calls WebPConfigPreset(), WebPValidateConfig(), WebPPictureInit(), WebPPictureImportRGB().
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
    enum { W = 8, H = 8 };
    uint8_t rgb[W * H * 3];
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            uint8_t *p = &rgb[(y * W + x) * 3];
            p[0] = (uint8_t)(x * 32);
            p[1] = (uint8_t)(y * 32);
            p[2] = (uint8_t)((x + y) * 16);
        }
    }

    WebPConfig config;
    assert(WebPConfigPreset(&config, WEBP_PRESET_DEFAULT, 75.0f) == 1);
    config.method = 4; /* mid-range speed/quality tradeoff */
    assert(WebPValidateConfig(&config) == 1);

    WebPPicture pic;
    assert(WebPPictureInit(&pic) == 1);
    pic.width = W;
    pic.height = H;
    pic.use_argb = 0; /* YUV path */
    assert(WebPPictureImportRGB(&pic, rgb, W * 3) == 1);

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic.writer = WebPMemoryWrite;
    pic.custom_ptr = &writer;

    int ok = WebPEncode(&config, &pic);
    assert(ok == 1);
    assert(writer.size > 0);
    assert(writer.mem != NULL);

    /* RIFF/WEBP magic: bytes 0..3 == "RIFF", 8..11 == "WEBP". */
    assert(writer.size >= 12);
    assert(memcmp(writer.mem + 0, "RIFF", 4) == 0);
    assert(memcmp(writer.mem + 8, "WEBP", 4) == 0);

    /* Confirm the bitstream decodes and reports the right dimensions. */
    int decoded_w = 0, decoded_h = 0;
    assert(WebPGetInfo(writer.mem, writer.size, &decoded_w, &decoded_h) == 1);
    assert(decoded_w == W);
    assert(decoded_h == H);

    WebPPictureFree(&pic);
    WebPMemoryWriterClear(&writer);

    printf("libwebp advanced-encode test OK\n");
    return 0;
}
