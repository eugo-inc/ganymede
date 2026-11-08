/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for thrift: compact; compiles against <thrift/transport/TBufferTransports.h>, <thrift/protocol/TCompactProtocol.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::protocol::TCompactProtocol;

    auto buffer = std::make_shared<TMemoryBuffer>();
    TCompactProtocol protocol(buffer);

    /* Values picked to exercise zigzag encoding: negative i32, large i64, byte. */
    const int8_t  b_in = -7;
    const int32_t i_in = -123456;          /* negative -> zigzag exercises sign bit  */
    const int64_t l_in = 0x0FEDCBA987654321LL;
    const std::string s_in = "compact protocol roundtrip";

    protocol.writeByte(b_in);
    protocol.writeI32(i_in);
    protocol.writeI64(l_in);
    protocol.writeString(s_in);

    uint8_t *raw = nullptr;
    uint32_t raw_len = 0;
    buffer->getBuffer(&raw, &raw_len);
    assert(raw_len > 0);

    int8_t  b_out = 0;
    int32_t i_out = 0;
    int64_t l_out = 0;
    std::string s_out;
    protocol.readByte(b_out);
    protocol.readI32(i_out);
    protocol.readI64(l_out);
    protocol.readString(s_out);

    assert(b_out == b_in);
    assert(i_out == i_in);
    assert(l_out == l_in);
    assert(s_out == s_in);

    std::printf("thrift compact OK: encoded=%u bytes b=%d i32=%d i64=0x%016llx s=\"%s\"\n",
                (unsigned)raw_len, (int)b_out, i_out,
                (unsigned long long)l_out, s_out.c_str());
    return 0;
}
