/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for thrift: json; compiles against <thrift/transport/TBufferTransports.h>, <thrift/protocol/TJSONProtocol.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::protocol::TJSONProtocol;
    using apache::thrift::protocol::TType;

    auto buffer = std::make_shared<TMemoryBuffer>();
    TJSONProtocol protocol(buffer);

    const int32_t i_in = 42;
    const int64_t l_in = -9876543210LL;
    const double  d_in = 3.141592653589793;
    const std::string s_in = "json protocol \"quoted\" + \\backslash";

    protocol.writeListBegin(apache::thrift::protocol::T_STRING, 4);
    protocol.writeI32(i_in);
    protocol.writeI64(l_in);
    protocol.writeDouble(d_in);
    protocol.writeString(s_in);
    protocol.writeListEnd();

    /* JSON protocol must produce printable ASCII on the wire — verify. */
    uint8_t *raw = nullptr;
    uint32_t raw_len = 0;
    buffer->getBuffer(&raw, &raw_len);
    assert(raw_len > 0);
    for (uint32_t i = 0; i < raw_len; ++i) {
        unsigned char c = raw[i];
        assert(c == '\t' || c == '\n' || c == '\r' || (c >= 0x20 && c <= 0x7e));
    }

    int32_t i_out = 0;
    int64_t l_out = 0;
    double  d_out = 0.0;
    std::string s_out;
    TType list_elem_type;
    uint32_t list_size = 0;
    protocol.readListBegin(list_elem_type, list_size);
    assert(list_size == 4);
    protocol.readI32(i_out);
    protocol.readI64(l_out);
    protocol.readDouble(d_out);
    protocol.readString(s_out);
    protocol.readListEnd();

    assert(i_out == i_in);
    assert(l_out == l_in);
    assert(std::fabs(d_out - d_in) < 1e-12);
    assert(s_out == s_in);

    std::printf("thrift json OK: wire=%u printable bytes i32=%d i64=%lld d=%.15g s=\"%s\"\n",
                (unsigned)raw_len, i_out, (long long)l_out, d_out, s_out.c_str());
    return 0;
}
