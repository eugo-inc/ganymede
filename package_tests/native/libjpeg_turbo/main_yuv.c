/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libjpeg_turbo: yuv; calls tjInitCompress(), tjBufSizeYUV2(), tjEncodeYUV3(), tjPlaneWidth().
 */

#include <turbojpeg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    enum { W = 64, H = 48 };
    const int pad = 4;
    const int subsamp = TJSAMP_420;

    unsigned char *src = (unsigned char *)malloc(W * H * 3);
    assert(src != NULL);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned char *p = &src[(y * W + x) * 3];
            p[0] = (unsigned char)(x * 4);
            p[1] = (unsigned char)(y * 4);
            p[2] = (unsigned char)((x + y) * 2);
        }
    }

    tjhandle h = tjInitCompress();
    assert(h != NULL);

    /* Size the YUV planar buffer for 4:2:0 subsampling with row padding. */
    unsigned long yuv_size = tjBufSizeYUV2(W, pad, H, subsamp);
    assert(yuv_size > 0);

    assert(yuv_size > (unsigned long)(W * H));
    assert(yuv_size < (unsigned long)(W * H * 3));

    unsigned char *yuv_buf = (unsigned char *)malloc(yuv_size);
    assert(yuv_buf != NULL);
    /* Pre-fill with a sentinel — encoder must overwrite it. */
    memset(yuv_buf, 0xAB, yuv_size);

    int rc = tjEncodeYUV3(h, src, W, 0, H, TJPF_RGB,
                          yuv_buf, pad, subsamp, TJFLAG_FASTDCT);
    assert(rc == 0);

    /* Sentinel must be gone — at least one byte must have been written by the
     * encoder. Scan the first Y-plane row, which must have been touched. */
    int touched = 0;
    for (unsigned long i = 0; i < yuv_size; i++) {
        if (yuv_buf[i] != 0xAB) { touched = 1; break; }
    }
    assert(touched);

    /* Stride helpers should agree on the same planar layout. */
    int y_w = tjPlaneWidth(0, W, subsamp);
    int y_h = tjPlaneHeight(0, H, subsamp);
    int c_w = tjPlaneWidth(1, W, subsamp);
    int c_h = tjPlaneHeight(1, H, subsamp);
    assert(y_w == W);
    assert(y_h == H);
    /* 4:2:0 chroma plane is half-size in each dimension. */
    assert(c_w == W / 2);
    assert(c_h == H / 2);

    tjDestroy(h);
    free(yuv_buf);
    free(src);

    printf("libjpeg_turbo YUV API OK\n");
    return 0;
}
