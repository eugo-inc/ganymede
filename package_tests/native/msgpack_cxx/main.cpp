/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for msgpack_cxx: core API; calls msgpack::pack(), msgpack::unpack().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <msgpack.hpp>
#include <cassert>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>

int main() {
    using Payload = std::tuple<int, std::string, std::vector<int>>;
    const Payload original{42, std::string("hello"), std::vector<int>{1, 2, 3}};

    msgpack::sbuffer buf;
    msgpack::pack(buf, original);
    assert(buf.size() > 0);

    msgpack::object_handle oh = msgpack::unpack(buf.data(), buf.size());
    msgpack::object obj = oh.get();

    Payload roundtrip;
    obj.convert(roundtrip);

    assert(std::get<0>(roundtrip) == 42);
    assert(std::get<1>(roundtrip) == "hello");
    assert(std::get<2>(roundtrip) == (std::vector<int>{1, 2, 3}));

    std::cout << "msgpack_cxx test OK: MSGPACK_VERSION=" << MSGPACK_VERSION << std::endl;
    return 0;
}
