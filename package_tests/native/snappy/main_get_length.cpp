/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for snappy: get length; calls snappy::Compress(), snappy::GetUncompressedLength(), snappy::IsValidCompressedBuffer(), snappy::Uncompress().
 */

#include <snappy.h>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    const std::string original =
        "Demo snappy frame-header probe. "
        "Demo snappy frame-header probe. "
        "Demo snappy frame-header probe. "
        "Demo snappy frame-header probe.";

    std::string compressed;
    snappy::Compress(original.data(), original.size(), &compressed);
    assert(!compressed.empty());

    /* 1. GetUncompressedLength on the full frame returns true + the EXACT original size. */
    size_t reported = 0;
    const bool got = snappy::GetUncompressedLength(
        compressed.data(), compressed.size(), &reported);
    assert(got);
    assert(reported == original.size());

    /* 2. Garbage data must be rejected by the validator. The first five bytes
     *    (0xFF x4, 0xAA) all have the varint continuation bit set, so the
     *    length prefix runs to six bytes, past snappy's five-byte maximum --
     *    it's a malformed frame and IsValidCompressedBuffer must say so. */
    const char garbage[] = {
        (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
        (char)0xAA, (char)0x55, (char)0xC3, (char)0x3C
    };
    const bool valid_garbage = snappy::IsValidCompressedBuffer(garbage, sizeof(garbage));
    assert(!valid_garbage);

    /* 3. Uncompress on garbage must return false (NOT crash, NOT UB). */
    std::string out;
    const bool uncompressed_ok = snappy::Uncompress(garbage, sizeof(garbage), &out);
    assert(!uncompressed_ok);

    assert(snappy::IsValidCompressedBuffer(compressed.data(), compressed.size()));

    std::cout << "snappy frame-header test OK: reported uncompressed length = "
              << reported << std::endl;
    return 0;
}
