/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for thrift: zlib; compiles against <thrift/transport/TBufferTransports.h>, <thrift/transport/TZlibTransport.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TZlibTransport.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::transport::TZlibTransport;

    std::string payload;
    payload.reserve(32 * 1024);
    for (int i = 0; i < 1024; ++i) {
        payload.append("the quick brown fox jumps over!!");  /* 32 bytes */
    }
    const uint32_t payload_len = static_cast<uint32_t>(payload.size());
    assert(payload_len == 32u * 1024u);

    /* Write side: payload -> zlib -> inner memory buffer. */
    auto inner = std::make_shared<TMemoryBuffer>();
    {
        TZlibTransport zlib_w(inner);
        zlib_w.write(reinterpret_cast<const uint8_t *>(payload.data()), payload_len);
        zlib_w.finish();
    }

    uint8_t *compressed = nullptr;
    uint32_t compressed_len = 0;
    inner->getBuffer(&compressed, &compressed_len);
    assert(compressed_len > 0);
    assert(compressed_len < payload_len);   /* compression must shrink this payload */

    /* Read side: hand a fresh memory buffer of compressed bytes to the zlib
     * reader, decompress, compare. */
    auto inner_r = std::make_shared<TMemoryBuffer>(compressed, compressed_len);
    TZlibTransport zlib_r(inner_r);

    std::string decoded;
    decoded.resize(payload_len);
    uint32_t got = 0;
    while (got < payload_len) {
        uint32_t n = zlib_r.read(reinterpret_cast<uint8_t *>(&decoded[got]),
                                 payload_len - got);
        assert(n > 0);   /* must make forward progress until payload_len bytes */
        got += n;
    }
    assert(got == payload_len);
    assert(std::memcmp(decoded.data(), payload.data(), payload_len) == 0);

    std::printf("thrift zlib OK: %u bytes -> %u bytes (%.1f%% of original) roundtrip OK\n",
                (unsigned)payload_len, (unsigned)compressed_len,
                100.0 * (double)compressed_len / (double)payload_len);
    return 0;
}
