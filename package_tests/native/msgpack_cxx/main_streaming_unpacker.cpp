/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for msgpack_cxx: streaming unpacker; feeds packed objects to msgpack::unpacker in 4-byte chunks (reserve_buffer, buffer_consumed, next).
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <msgpack.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

int main() {
    // Produce a buffer containing THREE back-to-back msgpack objects.
    msgpack::sbuffer stream;
    msgpack::pack(stream, 7);
    msgpack::pack(stream, std::string("demo"));
    msgpack::pack(stream, std::vector<int>{10, 20, 30});
    assert(stream.size() > 0);

    // Drain via the streaming unpacker — copy the buffer in chunks to make sure
    // the feed/reserve_buffer/buffer_consumed handshake actually works.
    msgpack::unpacker pac;
    const std::size_t chunk = 4;
    std::size_t offset = 0;
    while (offset < stream.size()) {
        const std::size_t take = std::min(chunk, stream.size() - offset);
        pac.reserve_buffer(take);
        std::memcpy(pac.buffer(), stream.data() + offset, take);
        pac.buffer_consumed(take);
        offset += take;
    }

    msgpack::object_handle oh;
    int count = 0;
    int got_int = -1;
    std::string got_str;
    std::vector<int> got_vec;

    while (pac.next(oh)) {
        msgpack::object obj = oh.get();
        if (count == 0) {
            obj.convert(got_int);
        } else if (count == 1) {
            obj.convert(got_str);
        } else if (count == 2) {
            obj.convert(got_vec);
        }
        ++count;
    }

    assert(count == 3);
    assert(got_int == 7);
    assert(got_str == "demo");
    assert(got_vec == (std::vector<int>{10, 20, 30}));

    return 0;
}
