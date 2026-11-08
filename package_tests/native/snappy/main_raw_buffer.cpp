/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for snappy: raw buffer; calls snappy::MaxCompressedLength(), snappy::RawCompress(), snappy::GetUncompressedLength(), snappy::RawUncompress().
 */

#include <snappy.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>

int main() {
    /* Repeating payload so the compressor produces something < input. */
    const std::string payload =
        "snappy raw buffer probe payload. snappy raw buffer probe payload. "
        "snappy raw buffer probe payload. snappy raw buffer probe payload. "
        "snappy raw buffer probe payload. snappy raw buffer probe payload.";

    const size_t in_len = payload.size();
    const char* in = payload.data();

    /* 1. MaxCompressedLength must give a safe upper bound. */
    const size_t bound = snappy::MaxCompressedLength(in_len);
    assert(bound >= in_len);  /* worst case must accommodate uncompressible data + overhead */

    char* compressed = static_cast<char*>(std::malloc(bound));
    assert(compressed != nullptr);

    /* 2. RawCompress writes the actual produced length back through out_len. */
    size_t produced_len = bound;
    snappy::RawCompress(in, in_len, compressed, &produced_len);
    assert(produced_len > 0);
    assert(produced_len <= bound);
    /* For a highly repetitive payload, compressed output must be strictly smaller. */
    assert(produced_len < in_len);

    /* 3. GetUncompressedLength reads the original size back from the frame header. */
    size_t parsed_uncompressed_len = 0;
    const bool got_len = snappy::GetUncompressedLength(
        compressed, produced_len, &parsed_uncompressed_len);
    assert(got_len);
    assert(parsed_uncompressed_len == in_len);

    /* 4. RawUncompress writes exactly parsed_uncompressed_len bytes to caller's buffer. */
    char* roundtrip = static_cast<char*>(std::malloc(parsed_uncompressed_len));
    assert(roundtrip != nullptr);
    const bool ok = snappy::RawUncompress(compressed, produced_len, roundtrip);
    assert(ok);
    assert(std::memcmp(roundtrip, in, in_len) == 0);

    std::free(compressed);
    std::free(roundtrip);

    std::cout << "snappy raw-buffer test OK: " << in_len << " -> "
              << produced_len << " bytes (bound " << bound << ")" << std::endl;
    return 0;
}
