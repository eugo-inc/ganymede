/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libwebp: demux; calls WebPEncodeLosslessRGB(), WebPDemux(), WebPDemuxGetI(), WebPDemuxGetFrame().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <webp/encode.h>
#include <webp/demux.h>
#include <webp/mux_types.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    static const uint8_t pixels[3 * 2 * 3] = {
        0xff, 0x00, 0x00,  0x00, 0xff, 0x00,  0x00, 0x00, 0xff,
        0x10, 0x20, 0x30,  0x40, 0x50, 0x60,  0x70, 0x80, 0x90,
    };

    uint8_t *encoded = NULL;
    size_t encoded_size = WebPEncodeLosslessRGB(pixels, 3, 2, 3 * 3, &encoded);
    assert(encoded_size > 0);
    assert(encoded != NULL);

    WebPData data = { encoded, encoded_size };
    WebPDemuxer *demux = WebPDemux(&data);
    assert(demux != NULL);

    uint32_t canvas_w = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    uint32_t canvas_h = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
    uint32_t frame_count = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);
    assert(canvas_w == 3);
    assert(canvas_h == 2);
    assert(frame_count == 1);

    WebPIterator iter;
    memset(&iter, 0, sizeof(iter));
    assert(WebPDemuxGetFrame(demux, 1, &iter) == 1);
    assert(iter.width == 3);
    assert(iter.height == 2);
    assert(iter.frame_num == 1);
    assert(iter.fragment.size > 0);
    assert(iter.fragment.bytes != NULL);

    WebPDemuxReleaseIterator(&iter);
    WebPDemuxDelete(demux);
    WebPFree(encoded);

    printf("libwebp demux test OK: demux_abi=0x%06x\n",
           WebPGetDemuxVersion());
    return 0;
}
