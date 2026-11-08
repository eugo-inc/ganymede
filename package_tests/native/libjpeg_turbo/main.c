/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libjpeg_turbo: core API; calls tjInitCompress(), tjCompress2(), tjDestroy(), tjInitDecompress().
 */

#include <turbojpeg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    enum { W = 16, H = 16 };
    unsigned char src[W * H * 3];
    for (int i = 0; i < W * H * 3; i++) {
        src[i] = (unsigned char)(i & 0xFF);
    }

    tjhandle compressor = tjInitCompress();
    assert(compressor != NULL);

    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;
    int rc = tjCompress2(compressor, src, W, 0, H, TJPF_RGB,
                         &jpeg_buf, &jpeg_size, TJSAMP_444, 90, TJFLAG_FASTDCT);
    assert(rc == 0);
    assert(jpeg_buf != NULL);
    assert(jpeg_size > 0);
    tjDestroy(compressor);

    tjhandle decompressor = tjInitDecompress();
    assert(decompressor != NULL);

    int width = 0, height = 0, subsamp = 0, colorspace = 0;
    rc = tjDecompressHeader3(decompressor, jpeg_buf, jpeg_size,
                             &width, &height, &subsamp, &colorspace);
    assert(rc == 0);
    assert(width == W);
    assert(height == H);
    assert(subsamp == TJSAMP_444);
    tjDestroy(decompressor);

    tjFree(jpeg_buf);

    printf("libjpeg_turbo test OK\n");
    return 0;
}
