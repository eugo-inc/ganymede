/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for thrift: multiplexed; compiles against <thrift/transport/TBufferTransports.h>, <thrift/protocol/TBinaryProtocol.h>, <thrift/protocol/TMultiplexedProtocol.h>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/protocol/TProtocol.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::protocol::TBinaryProtocol;
    using apache::thrift::protocol::TMultiplexedProtocol;
    using apache::thrift::protocol::TMessageType;
    using apache::thrift::protocol::T_CALL;

    auto buffer = std::make_shared<TMemoryBuffer>();
    auto binary = std::make_shared<TBinaryProtocol>(buffer);

    /* The decorator delegates everything except writeMessageBegin to `binary`. */
    TMultiplexedProtocol multiplexed(binary, "Calc");

    const std::string method = "add";
    const int32_t seqid = 7;
    multiplexed.writeMessageBegin(method, T_CALL, seqid);
    multiplexed.writeI32(123);
    multiplexed.writeMessageEnd();

    /* Read the header back through the underlying binary protocol. The on-wire
     * name MUST be "Calc:add" — that's the entire point of TMultiplexedProtocol. */
    std::string wire_name;
    TMessageType wire_type;
    int32_t wire_seqid = 0;
    binary->readMessageBegin(wire_name, wire_type, wire_seqid);

    assert(wire_name == "Calc:add");
    assert(wire_type == T_CALL);
    assert(wire_seqid == seqid);

    int32_t payload_out = 0;
    binary->readI32(payload_out);
    binary->readMessageEnd();
    assert(payload_out == 123);

    std::printf("thrift multiplexed OK: wire name=\"%s\" type=%d seqid=%d payload=%d\n",
                wire_name.c_str(), (int)wire_type, wire_seqid, payload_out);
    return 0;
}
