/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for thrift: framed; compiles against <thrift/transport/TBufferTransports.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::transport::TFramedTransport;

    const std::string payload = "framed-transport-payload-12345678";
    const uint32_t payload_len = static_cast<uint32_t>(payload.size());

    /* Write side: payload -> framed -> inner. flush() emits the size-prefixed
     * frame into the underlying transport. */
    auto inner = std::make_shared<TMemoryBuffer>();
    {
        TFramedTransport framed_w(inner);
        framed_w.write(reinterpret_cast<const uint8_t *>(payload.data()), payload_len);
        framed_w.flush();
    }

    /* Inspect the on-wire bytes — must be exactly 4 (length) + payload_len. */
    uint8_t *wire = nullptr;
    uint32_t wire_len = 0;
    inner->getBuffer(&wire, &wire_len);
    assert(wire_len == payload_len + 4u);

    /* The 4-byte prefix is big-endian network order. */
    const uint32_t frame_len =
        (static_cast<uint32_t>(wire[0]) << 24) |
        (static_cast<uint32_t>(wire[1]) << 16) |
        (static_cast<uint32_t>(wire[2]) <<  8) |
        (static_cast<uint32_t>(wire[3]));
    assert(frame_len == payload_len);

    /* Payload bytes must follow verbatim. */
    assert(std::memcmp(wire + 4, payload.data(), payload_len) == 0);

    /* Read side: hand the framed bytes to a fresh framed transport and read
     * them back. */
    auto inner_r = std::make_shared<TMemoryBuffer>(wire, wire_len);
    TFramedTransport framed_r(inner_r);

    std::string decoded;
    decoded.resize(payload_len);
    uint32_t got = framed_r.readAll(reinterpret_cast<uint8_t *>(&decoded[0]),
                                    payload_len);
    assert(got == payload_len);
    assert(decoded == payload);

    std::printf("thrift framed OK: payload=%u bytes wire=%u bytes (4-byte BE prefix)\n",
                (unsigned)payload_len, (unsigned)wire_len);
    return 0;
}
