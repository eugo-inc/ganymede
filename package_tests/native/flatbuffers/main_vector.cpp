/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for flatbuffers: vector; compiles against <flatbuffers/flatbuffers.h>.
 */

#include <flatbuffers/flatbuffers.h>
#include <cassert>
#include <cstdio>
#include <cstdint>

int main() {
    flatbuffers::FlatBufferBuilder builder(1024);

    const int32_t source[] = {-2147483647 - 1, -1, 0, 1, 42, 2147483647};
    const size_t source_n = sizeof(source) / sizeof(source[0]);

    auto vec_off = builder.CreateVector(source, source_n);
    builder.Finish(vec_off);

    const uint8_t *buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    assert(buf != nullptr);
    assert(size > 0);

    /* Read back as a Vector<int32_t> root. */
    const flatbuffers::Vector<int32_t> *root =
        flatbuffers::GetRoot<flatbuffers::Vector<int32_t>>(buf);
    assert(root != nullptr);
    assert(root->size() == source_n);

    for (size_t i = 0; i < source_n; ++i) {
        assert(root->Get(static_cast<flatbuffers::uoffset_t>(i)) == source[i]);
    }

    std::printf("flatbuffers vector test OK: %zu elements round-tripped\n", source_n);
    return 0;
}
