/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtiff: core API; calls mkstemps(), TIFFOpen(), TIFFSetField(), TIFFWriteEncodedStrip().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <tiffio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libtiff_test_XXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

    const uint32_t W = 4, H = 4;
    unsigned char src[4 * 4 * 3];
    for (size_t i = 0; i < sizeof(src); i++) src[i] = (unsigned char)(i & 0xFF);

    TIFF *out = TIFFOpen(path, "w");
    assert(out != NULL);
    assert(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, W) == 1);
    assert(TIFFSetField(out, TIFFTAG_IMAGELENGTH, H) == 1);
    assert(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8) == 1);
    assert(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3) == 1);
    assert(TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, H) == 1);
    assert(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) == 1);
    assert(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1);
    assert(TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE) == 1);
    assert(TIFFWriteEncodedStrip(out, 0, src, sizeof(src)) == (tmsize_t)sizeof(src));
    TIFFClose(out);

    TIFF *in = TIFFOpen(path, "r");
    assert(in != NULL);
    uint32_t w = 0, h = 0;
    assert(TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w) == 1);
    assert(TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h) == 1);
    assert(w == W && h == H);
    unsigned char back[sizeof(src)];
    assert(TIFFReadEncodedStrip(in, 0, back, sizeof(back)) == (tmsize_t)sizeof(back));
    assert(memcmp(src, back, sizeof(src)) == 0);
    TIFFClose(in);

    unlink(path);

    printf("libtiff test OK: %s\n", TIFFGetVersion());
    return 0;
}
