/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for msgpack_cxx: define macro; calls msgpack::pack(), msgpack::unpack().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <msgpack.hpp>
#include <cassert>
#include <string>
#include <vector>

struct Sample {
    int id;
    std::string label;
    std::vector<double> samples;

    MSGPACK_DEFINE(id, label, samples);
};

int main() {
    Sample original;
    original.id = 1729;
    original.label = "ramanujan";
    original.samples = {1.5, 2.5, 3.5};

    msgpack::sbuffer buf;
    msgpack::pack(buf, original);
    assert(buf.size() > 0);

    msgpack::object_handle oh = msgpack::unpack(buf.data(), buf.size());
    msgpack::object obj = oh.get();

    // MSGPACK_DEFINE serializes a struct as a msgpack array of its fields,
    // in declaration order. Assert that wire shape before converting back.
    assert(obj.type == msgpack::type::ARRAY);
    assert(obj.via.array.size == 3);

    Sample roundtrip;
    obj.convert(roundtrip);

    assert(roundtrip.id == 1729);
    assert(roundtrip.label == "ramanujan");
    assert(roundtrip.samples.size() == 3);
    assert(roundtrip.samples[0] == 1.5);
    assert(roundtrip.samples[1] == 2.5);
    assert(roundtrip.samples[2] == 3.5);

    return 0;
}
