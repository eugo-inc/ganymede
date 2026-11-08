/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for protobuf: coded stream; calls CodedOutputStream::WriteVarint32/64(), WriteTag(),
 * WriteLittleEndian32/64(), CodedInputStream::ReadVarint32/64(), ReadTag(), ReadLittleEndian32/64().
 */

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/stubs/common.h>  /* ShutdownProtobufLibrary() */
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

int main() {
    std::string buffer;

    /* Encode: a varint, a tag, a fixed32, and a fixed64 into a string. */
    {
        google::protobuf::io::StringOutputStream raw_out(&buffer);
        google::protobuf::io::CodedOutputStream coded_out(&raw_out);

        coded_out.WriteVarint32(150u);           /* small varint, 2-byte encoding */
        coded_out.WriteVarint64(0xDEADBEEFCAFEull);
        coded_out.WriteTag((5u << 3) | 2u);      /* field 5, wire type LEN */
        coded_out.WriteLittleEndian32(0x12345678u);
        coded_out.WriteLittleEndian64(0xAABBCCDD11223344ull);

        /* coded_out destructor flushes to the StringOutputStream. */
    }
    assert(!buffer.empty());

    {
        google::protobuf::io::ArrayInputStream raw_in(buffer.data(),
                                                     static_cast<int>(buffer.size()));
        google::protobuf::io::CodedInputStream coded_in(&raw_in);

        uint32_t v32 = 0;
        bool ok = coded_in.ReadVarint32(&v32);
        assert(ok);
        assert(v32 == 150u);

        uint64_t v64 = 0;
        ok = coded_in.ReadVarint64(&v64);
        assert(ok);
        assert(v64 == 0xDEADBEEFCAFEull);

        uint32_t tag = coded_in.ReadTag();
        assert(tag == ((5u << 3) | 2u));

        uint32_t f32 = 0;
        ok = coded_in.ReadLittleEndian32(&f32);
        assert(ok);
        assert(f32 == 0x12345678u);

        uint64_t f64 = 0;
        ok = coded_in.ReadLittleEndian64(&f64);
        assert(ok);
        assert(f64 == 0xAABBCCDD11223344ull);

        uint32_t trailing = 0;
        bool more = coded_in.ReadVarint32(&trailing);
        assert(!more);
    }

    std::printf("protobuf coded_stream test OK: encoded_bytes=%zu\n", buffer.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
