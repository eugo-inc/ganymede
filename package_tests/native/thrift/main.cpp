/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for thrift: core API; compiles against <thrift/transport/TBufferTransports.h>, <thrift/protocol/TBinaryProtocol.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::protocol::TBinaryProtocol;

    auto buffer = std::make_shared<TMemoryBuffer>();
    TBinaryProtocol protocol(buffer);

    /* Serialize a representative mix of primitives + a string. */
    const int32_t i_in = 0x01020304;
    const int64_t l_in = 0x1122334455667788LL;
    const std::string s_in = "hello thrift";
    protocol.writeI32(i_in);
    protocol.writeI64(l_in);
    protocol.writeString(s_in);

    int32_t i_out = 0;
    int64_t l_out = 0;
    std::string s_out;
    protocol.readI32(i_out);
    protocol.readI64(l_out);
    protocol.readString(s_out);

    assert(i_out == i_in);
    assert(l_out == l_in);
    assert(s_out == s_in);

    std::printf("thrift test OK: roundtrip i32=0x%08x i64=0x%016llx s=\"%s\"\n",
                i_out, (unsigned long long)l_out, s_out.c_str());
    return 0;
}
