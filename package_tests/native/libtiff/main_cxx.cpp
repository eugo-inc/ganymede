/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtiff: cxx; calls TIFFStreamOpen(), TIFFSetField(), TIFFWriteEncodedStrip(), TIFFClose().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <tiffio.h>
#include <tiffio.hxx>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

int main() {
    const uint32_t W = 8, H = 8;
    unsigned char src[8 * 8 * 3];
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = static_cast<unsigned char>((i * 3 + 5) & 0xFF);
    }

    /* Encode into an in-memory ostringstream via the C++ binding. */
    std::ostringstream out_stream(std::ios::out | std::ios::binary);
    TIFF *out = TIFFStreamOpen("demo_mem_writer", &out_stream);
    assert(out != nullptr);
    assert(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, W) == 1);
    assert(TIFFSetField(out, TIFFTAG_IMAGELENGTH, H) == 1);
    assert(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8) == 1);
    assert(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3) == 1);
    assert(TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, H) == 1);
    assert(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) == 1);
    assert(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1);
    assert(TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE) == 1);
    assert(TIFFWriteEncodedStrip(out, 0, src, sizeof(src))
           == static_cast<tmsize_t>(sizeof(src)));
    TIFFClose(out);

    const std::string encoded = out_stream.str();
    assert(encoded.size() > 8 + sizeof(src));

    /* Decode from an in-memory istringstream. */
    std::istringstream in_stream(encoded, std::ios::in | std::ios::binary);
    TIFF *in = TIFFStreamOpen("demo_mem_reader", &in_stream);
    assert(in != nullptr);
    uint32_t w = 0, h = 0;
    assert(TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w) == 1);
    assert(TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h) == 1);
    assert(w == W && h == H);
    unsigned char back[sizeof(src)];
    assert(TIFFReadEncodedStrip(in, 0, back, sizeof(back))
           == static_cast<tmsize_t>(sizeof(back)));
    assert(std::memcmp(src, back, sizeof(src)) == 0);
    TIFFClose(in);

    std::printf("libtiffxx stream-API test OK\n");
    return 0;
}
