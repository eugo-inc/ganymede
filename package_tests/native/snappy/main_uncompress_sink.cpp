/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for snappy: uncompress sink; calls snappy::MaxCompressedLength(), snappy::RawCompress(), snappy::GetUncompressedLength(), snappy::Uncompress().
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
        "Demo snappy Uncompress(Source*, Sink*) probe payload. "
        "Demo snappy Uncompress(Source*, Sink*) probe payload. "
        "Demo snappy Uncompress(Source*, Sink*) probe payload. "
        "Demo snappy Uncompress(Source*, Sink*) probe payload.";

    /* 1. Produce a compressed frame with RawCompress. */
    const size_t bound = snappy::MaxCompressedLength(original.size());
    char* compressed = static_cast<char*>(std::malloc(bound));
    assert(compressed != nullptr);
    size_t compressed_len = bound;
    snappy::RawCompress(original.data(), original.size(), compressed, &compressed_len);
    assert(compressed_len > 0);
    assert(compressed_len < original.size());  /* repetitive payload must compress */

    /* 2. Wrap the compressed frame as a ByteArraySource. */
    snappy::ByteArraySource source(compressed, compressed_len);
    assert(source.Available() == compressed_len);

    /* 3. Size the output buffer from the frame header (independent oracle). */
    size_t uncompressed_len = 0;
    const bool got_len = snappy::GetUncompressedLength(
        compressed, compressed_len, &uncompressed_len);
    assert(got_len);
    assert(uncompressed_len == original.size());

    /* 4. Wrap the output buffer in a Sink and run Uncompress(Source*, Sink*). */
    char* out_buf = static_cast<char*>(std::malloc(uncompressed_len));
    assert(out_buf != nullptr);
    snappy::UncheckedByteArraySink sink(out_buf);

    const bool ok = snappy::Uncompress(&source, &sink);
    assert(ok);

    /* 5. Decompressed bytes must match the original byte-for-byte. */
    assert(std::memcmp(out_buf, original.data(), original.size()) == 0);

    std::free(compressed);
    std::free(out_buf);

    std::cout << "snappy Uncompress(Source*, Sink*) test OK: "
              << compressed_len << " -> " << uncompressed_len << " bytes" << std::endl;
    return 0;
}
