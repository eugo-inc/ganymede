/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for boost: iostreams; calls io::gzip_compressor(), io::back_inserter(), io::copy(), io::gzip_decompressor().
 */

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace io = boost::iostreams;

int main() {
    const std::string original =
        "demo boost iostreams gzip round-trip — "
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog.";

    /* Compress: feed `original` through gzip_compressor into `compressed`. */
    std::vector<char> compressed;
    {
        io::filtering_streambuf<io::output> out;
        out.push(io::gzip_compressor());
        out.push(io::back_inserter(compressed));
        io::array_source src(original.data(), original.size());
        io::copy(src, out);
    }
    assert(!compressed.empty());
    /* gzip magic header: 0x1f 0x8b. Proves the codec ran. */
    assert(compressed.size() >= 2);
    assert(static_cast<std::uint8_t>(compressed[0]) == 0x1f);
    assert(static_cast<std::uint8_t>(compressed[1]) == 0x8b);
    /* Highly redundant input must compress below original size. */
    assert(compressed.size() < original.size());

    /* Decompress: feed `compressed` through gzip_decompressor into `roundtrip`. */
    std::vector<char> roundtrip;
    {
        io::filtering_streambuf<io::output> out;
        out.push(io::gzip_decompressor());
        out.push(io::back_inserter(roundtrip));
        io::array_source src(compressed.data(), compressed.size());
        io::copy(src, out);
    }

    const std::string recovered(roundtrip.begin(), roundtrip.end());
    assert(recovered == original);

    std::printf("boost iostreams gzip test OK (%zu -> %zu bytes)\n",
                original.size(), compressed.size());
    return 0;
}
