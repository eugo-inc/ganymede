/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openexr: rgba io; calls IMF::Rgba(), half().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImathBox.h>
#include <half.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

int main(void) {
    namespace IMF = OPENEXR_IMF_NAMESPACE;
    namespace IMATH = IMATH_NAMESPACE;

    const int W = 4;
    const int H = 3;

    char path[64];
    std::snprintf(path, sizeof(path), "/tmp/demo_openexr_probe_%d.exr",
                  static_cast<int>(getpid()));

    IMF::Array2D<IMF::Rgba> out_pixels(H, W);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            float v = static_cast<float>(y * W + x);
            out_pixels[y][x] = IMF::Rgba(half(v),
                                         half(v + 0.5f),
                                         half(v + 1.0f),
                                         half(1.0f));
        }
    }

    /* 2. Write the EXR (default compression = ZIP, exercises libdeflate). */
    {
        IMF::RgbaOutputFile out(path, W, H, IMF::WRITE_RGBA);
        out.setFrameBuffer(&out_pixels[0][0], 1, W);
        out.writePixels(H);
    }

    /* 3. Read it back. */
    IMF::Array2D<IMF::Rgba> in_pixels(H, W);
    {
        IMF::RgbaInputFile in(path);
        IMATH::Box2i dw = in.dataWindow();
        int rw = dw.max.x - dw.min.x + 1;
        int rh = dw.max.y - dw.min.y + 1;
        assert(rw == W);
        assert(rh == H);

        in.setFrameBuffer(&in_pixels[0][0] - dw.min.x - dw.min.y * W, 1, W);
        in.readPixels(dw.min.y, dw.max.y);
    }

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const IMF::Rgba& a = out_pixels[y][x];
            const IMF::Rgba& b = in_pixels[y][x];
            assert(a.r.bits() == b.r.bits());
            assert(a.g.bits() == b.g.bits());
            assert(a.b.bits() == b.b.bits());
            assert(a.a.bits() == b.a.bits());
        }
    }

    /* 5. Clean up. */
    std::remove(path);

    std::cout << "openexr rgba_io test OK (" << W << "x" << H << ")" << std::endl;
    return 0;
}
