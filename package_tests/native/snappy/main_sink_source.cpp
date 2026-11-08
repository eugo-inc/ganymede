/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for snappy: sink source; calls snappy::MaxCompressedLength(), snappy::Compress(), snappy::Uncompress().
 */

#include <snappy.h>
#include <snappy-sinksource.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

int main() {
    const std::string original =
        "Demo snappy Source/Sink probe payload. "
        "Demo snappy Source/Sink probe payload. "
        "Demo snappy Source/Sink probe payload. "
        "Demo snappy Source/Sink probe payload.";

    /* 1. Wrap the input in a ByteArraySource. */
    snappy::ByteArraySource source(original.data(), original.size());
    assert(source.Available() == original.size());

    /* 2. Allocate a worst-case output buffer and wrap in an UncheckedByteArraySink. */
    const size_t bound = snappy::MaxCompressedLength(original.size());
    char* out_buf = static_cast<char*>(std::malloc(bound));
    assert(out_buf != nullptr);
    snappy::UncheckedByteArraySink sink(out_buf);

    /* 3. Compress through the Source/Sink overload. */
    const size_t produced = snappy::Compress(&source, &sink);
    assert(produced > 0);
    assert(produced <= bound);
    assert(produced < original.size());  /* repetitive payload must compress */

    /* 4. Cross-API check: the std::string Uncompress overload must accept output
     *    that was produced via the Source/Sink Compress overload (same engine). */
    std::string roundtrip;
    const bool ok = snappy::Uncompress(out_buf, produced, &roundtrip);
    assert(ok);
    assert(roundtrip.size() == original.size());
    assert(std::memcmp(roundtrip.data(), original.data(), original.size()) == 0);

    std::free(out_buf);

    std::cout << "snappy Source/Sink test OK: " << original.size() << " -> "
              << produced << " bytes" << std::endl;
    return 0;
}
