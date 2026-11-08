/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for snappy: core API; calls snappy::Compress(), snappy::IsValidCompressedBuffer(), snappy::Uncompress().
 */

#include <snappy.h>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    /* Use repeating-ish input so the compressor actually finds something to compress. */
    const std::string original =
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog.";

    std::string compressed;
    const size_t produced = snappy::Compress(original.data(), original.size(), &compressed);
    assert(produced == compressed.size());
    assert(!compressed.empty());

    assert(snappy::IsValidCompressedBuffer(compressed.data(), compressed.size()));

    std::string roundtrip;
    const bool ok = snappy::Uncompress(compressed.data(), compressed.size(), &roundtrip);
    assert(ok);
    assert(roundtrip == original);

    std::cout << "snappy test OK: " << original.size() << " -> "
              << compressed.size() << " bytes" << std::endl;
    return 0;
}
