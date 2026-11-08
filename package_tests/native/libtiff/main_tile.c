/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libtiff: tile; calls mkstemps(), TIFFOpen(), TIFFSetField(), TIFFTileSize().
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
    char path[] = "/tmp/demo_libtiff_tile_XXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

    const uint32_t W = 32, H = 32, TW = 16, TH = 16;
    const size_t TILE_BYTES = (size_t)TW * TH;

    unsigned char src[32 * 32];
    for (size_t i = 0; i < sizeof(src); i++) src[i] = (unsigned char)(i * 7 + 13);

    TIFF *out = TIFFOpen(path, "w");
    assert(out != NULL);
    assert(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, W) == 1);
    assert(TIFFSetField(out, TIFFTAG_IMAGELENGTH, H) == 1);
    assert(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8) == 1);
    assert(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1) == 1);
    assert(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK) == 1);
    assert(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1);
    assert(TIFFSetField(out, TIFFTAG_TILEWIDTH, TW) == 1);
    assert(TIFFSetField(out, TIFFTAG_TILELENGTH, TH) == 1);
    assert(TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE) == 1);

    tmsize_t tile_size = TIFFTileSize(out);
    assert(tile_size == (tmsize_t)TILE_BYTES);

    /* Pack each 16x16 quadrant into a tile buffer and write. */
    unsigned char tile_buf[16 * 16];
    for (uint32_t ty = 0; ty < H; ty += TH) {
        for (uint32_t tx = 0; tx < W; tx += TW) {
            for (uint32_t y = 0; y < TH; y++) {
                memcpy(&tile_buf[y * TW], &src[(ty + y) * W + tx], TW);
            }
            assert(TIFFWriteEncodedTile(out, TIFFComputeTile(out, tx, ty, 0, 0),
                                        tile_buf, tile_size) == tile_size);
        }
    }
    TIFFClose(out);

    /* Read back. */
    TIFF *in = TIFFOpen(path, "r");
    assert(in != NULL);
    /* Verify the file actually claims to be tiled. */
    assert(TIFFIsTiled(in) != 0);
    assert(TIFFNumberOfTiles(in) == 4);

    unsigned char back[32 * 32];
    memset(back, 0, sizeof(back));
    for (uint32_t ty = 0; ty < H; ty += TH) {
        for (uint32_t tx = 0; tx < W; tx += TW) {
            assert(TIFFReadEncodedTile(in, TIFFComputeTile(in, tx, ty, 0, 0),
                                       tile_buf, tile_size) == tile_size);
            for (uint32_t y = 0; y < TH; y++) {
                memcpy(&back[(ty + y) * W + tx], &tile_buf[y * TW], TW);
            }
        }
    }
    TIFFClose(in);

    assert(memcmp(src, back, sizeof(src)) == 0);

    unlink(path);

    printf("libtiff tile-API test OK\n");
    return 0;
}
