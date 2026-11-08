/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for msgpack_cxx: packer and zone; builds a map with msgpack::packer and unpacks it into a msgpack::zone.
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <msgpack.hpp>
#include <cassert>
#include <cstring>
#include <map>
#include <string>

int main() {
    msgpack::sbuffer buf;
    msgpack::packer<msgpack::sbuffer> pk(&buf);
    // 1) Pack a two-entry map with the low-level packer.
    pk.pack_map(2);
    pk.pack(std::string("alpha"));
    pk.pack(1);
    pk.pack(std::string("beta"));
    pk.pack(2);
    assert(buf.size() > 0);

    // 2) Decode against a zone (arena-allocated object instead of object_handle).
    msgpack::zone z;
    msgpack::object obj = msgpack::unpack(z, buf.data(), buf.size());

    assert(obj.type == msgpack::type::MAP);
    assert(obj.via.map.size == 2);

    std::map<std::string, int> roundtrip;
    obj.convert(roundtrip);

    assert(roundtrip.size() == 2);
    assert(roundtrip.at("alpha") == 1);
    assert(roundtrip.at("beta") == 2);

    // 3) Verify the zone-backed object remains valid after the source buffer
    //    is mutated — unpack(zone, ...) deep-copies into the zone's arena, so
    //    overwriting `buf` must NOT corrupt `obj`. (object_handle has the same
    //    guarantee via its embedded zone; this confirms the explicit-zone form.)
    std::memset(buf.data(), 0, buf.size());
    assert(obj.type == msgpack::type::MAP);
    assert(obj.via.map.size == 2);

    return 0;
}
