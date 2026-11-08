/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for thrift: exception; compiles against <thrift/transport/TBufferTransports.h>, <thrift/transport/TTransportException.h>, <exception>.
 */

#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportException.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <memory>
#include <string>

int main() {
    using apache::thrift::transport::TMemoryBuffer;
    using apache::thrift::transport::TTransportException;

    const uint8_t one_byte[1] = { 0x42 };
    auto buffer = std::make_shared<TMemoryBuffer>(
        const_cast<uint8_t *>(one_byte), 1u, TMemoryBuffer::COPY);

    /* Sanity: a single-byte read succeeds. */
    uint8_t got_byte = 0;
    uint32_t n = buffer->read(&got_byte, 1);
    assert(n == 1);
    assert(got_byte == 0x42);

    /* Now demand more bytes than the buffer holds. readAll() (not read()) is
     * the API contract that throws on under-read. */
    uint8_t scratch[8] = {0};
    bool caught_specific = false;
    bool caught_as_std = false;
    int kind_seen = -1;

    try {
        buffer->readAll(scratch, sizeof(scratch));
    } catch (const TTransportException &te) {
        caught_specific = true;
        kind_seen = te.getType();
        /* Re-throw and recatch as std::exception to verify inheritance. */
        try {
            throw te;
        } catch (const std::exception &e) {
            caught_as_std = true;
            /* what() must return a non-null, non-empty string per std::exception. */
            const char *msg = e.what();
            assert(msg != nullptr);
            assert(msg[0] != '\0');
        }
    }

    assert(caught_specific);
    assert(caught_as_std);
    /* TTransportException::END_OF_FILE is the canonical kind for under-read. */
    assert(kind_seen == TTransportException::END_OF_FILE);

    std::printf("thrift exception OK: caught TTransportException kind=%d (END_OF_FILE) via both concrete and std::exception\n",
                kind_seen);
    return 0;
}
