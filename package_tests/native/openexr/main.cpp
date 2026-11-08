/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openexr: headers and libraries agree; calls exr_get_library_version(), IMF::isImfMagic().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <OpenEXR/OpenEXRConfig.h>
#include <openexr.h>
#include <ImfVersion.h>
#include <cassert>
#include <iostream>

int main(void) {
    namespace IMF = OPENEXR_IMF_NAMESPACE;

    /* The library loaded at run time must be the release the headers describe; a stale
       or mixed install shows up here as a major/minor mismatch. */
    int major = -1;
    int minor = -1;
    int patch = -1;
    const char* extra = nullptr;
    exr_get_library_version(&major, &minor, &patch, &extra);
    assert(major == OPENEXR_VERSION_MAJOR);
    assert(minor == OPENEXR_VERSION_MINOR);

    /* The C++ library recognises the four-byte EXR magic number (20000630, little-endian)
       and rejects other file signatures. */
    const char exr_magic[4] = {0x76, 0x2f, 0x31, 0x01};
    const char zip_magic[4] = {'P', 'K', 0x03, 0x04};
    assert(IMF::isImfMagic(exr_magic));
    assert(!IMF::isImfMagic(zip_magic));

    std::cout << "openexr test OK: headers " << OPENEXR_VERSION_STRING << ", library "
              << major << "." << minor << "." << patch << std::endl;
    return 0;
}
