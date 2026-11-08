/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libjpeg_turbo: transform; calls tjInitCompress(), tjCompress2(), tjDestroy(), tjInitTransform().
 */

#include <turbojpeg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Use dimensions that are multiples of the MCU (16 for 4:2:0) so the
     * rotate-90 transform is exact and no PERFECT-flag warnings arise. */
    enum { W = 32, H = 16 };
    unsigned char src[W * H * 3];
    for (int i = 0; i < W * H * 3; i++) {
        src[i] = (unsigned char)((i * 13) & 0xFF);
    }

    tjhandle compressor = tjInitCompress();
    assert(compressor != NULL);
    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;
    int rc = tjCompress2(compressor, src, W, 0, H, TJPF_RGB,
                         &jpeg_buf, &jpeg_size, TJSAMP_420, 80, TJFLAG_FASTDCT);
    assert(rc == 0);
    assert(jpeg_buf != NULL);
    assert(jpeg_size > 0);
    tjDestroy(compressor);

    tjhandle xform = tjInitTransform();
    assert(xform != NULL);

    tjtransform t;
    memset(&t, 0, sizeof(t));
    t.op = TJXOP_ROT90;
    t.options = TJXOPT_PERFECT;

    unsigned char *xformed = NULL;
    unsigned long xformed_size = 0;
    rc = tjTransform(xform, jpeg_buf, jpeg_size,
                     1, &xformed, &xformed_size, &t, TJFLAG_FASTDCT);
    assert(rc == 0);
    assert(xformed != NULL);
    assert(xformed_size > 0);
    assert(xformed[0] == 0xFF);
    assert(xformed[1] == 0xD8);
    tjDestroy(xform);

    tjhandle decompressor = tjInitDecompress();
    assert(decompressor != NULL);

    int rw = 0, rh = 0, rsubsamp = 0, rcolorspace = 0;
    rc = tjDecompressHeader3(decompressor, xformed, xformed_size,
                             &rw, &rh, &rsubsamp, &rcolorspace);
    assert(rc == 0);
    assert(rw == H);  /* width and height swapped by ROT90 */
    assert(rh == W);
    tjDestroy(decompressor);

    tjFree(xformed);
    tjFree(jpeg_buf);

    printf("libjpeg_turbo transform API OK\n");
    return 0;
}
