/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openexr: header; calls IMF::Channel(), IMF::IntAttribute().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfCompressionAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfPixelType.h>
#include <ImathBox.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

int main(void) {
    namespace IMF = OPENEXR_IMF_NAMESPACE;
    namespace IMATH = IMATH_NAMESPACE;

    const int W = 8;
    const int H = 6;
    IMF::Header h(W, H, /*pixelAspectRatio=*/1.0f);

    /* The display window should be [0,0]..[W-1,H-1]. */
    const IMATH::Box2i& dw = h.displayWindow();
    assert(dw.min.x == 0 && dw.min.y == 0);
    assert(dw.max.x == W - 1 && dw.max.y == H - 1);

    /* The data window initially equals the display window. */
    const IMATH::Box2i& data = h.dataWindow();
    assert(data.min == dw.min && data.max == dw.max);

    /* 2. Insert RGBA half-float channels — exercises ChannelList::insert. */
    IMF::ChannelList& chans = h.channels();
    chans.insert("R", IMF::Channel(IMF::HALF));
    chans.insert("G", IMF::Channel(IMF::HALF));
    chans.insert("B", IMF::Channel(IMF::HALF));
    chans.insert("A", IMF::Channel(IMF::HALF));

    /* Lookup via findChannel — returns nullptr if missing. */
    const IMF::Channel* cr = chans.findChannel("R");
    const IMF::Channel* cz = chans.findChannel("Z_not_present");
    assert(cr != NULL);
    assert(cr->type == IMF::HALF);
    assert(cz == NULL);

    h.compression() = IMF::ZIPS_COMPRESSION;
    assert(h.compression() == IMF::ZIPS_COMPRESSION);
    h.compression() = IMF::NO_COMPRESSION;
    assert(h.compression() == IMF::NO_COMPRESSION);

    /* 4. Generic typed-attribute insert via the IntAttribute path. */
    const char* attr_name = "demo_probe_int";
    h.insert(attr_name, IMF::IntAttribute(42));

    /* findTypedAttribute returns NULL when the type doesn't match — proves
     * the runtime-type check is wired up, not just blind cast. */
    const IMF::IntAttribute* got =
        h.findTypedAttribute<IMF::IntAttribute>(attr_name);
    assert(got != NULL);
    assert(got->value() == 42);

    /* 5. sanityCheck() — Header's own invariant validator. Must not throw on
     *    a fully-formed RGBA scanline header. The bool flags select tiled
     *    vs scanline + multipart; defaults are fine here. */
    h.sanityCheck(/*isTiled=*/false, /*isMultipartFile=*/false);

    std::cout << "openexr header test OK (chans="
              << (cr->type == IMF::HALF ? "HALF" : "?") << ")" << std::endl;
    return 0;
}
