/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for flatbuffers: core API; calls flatbuffers::GetRoot().
 */

#include <flatbuffers/flatbuffers.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    static_assert(FLATBUFFERS_VERSION_MAJOR >= 1, "flatbuffers major version missing");

    flatbuffers::FlatBufferBuilder builder(1024);
    const char *payload = "demo-flatbuffers-smoke";
    auto str_off = builder.CreateString(payload);
    builder.Finish(str_off);

    /* Buffer must be non-empty and pointer must be valid. */
    const uint8_t *buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    assert(buf != nullptr);
    assert(size > 0);

    const flatbuffers::String *root = flatbuffers::GetRoot<flatbuffers::String>(buf);
    assert(root != nullptr);
    assert(std::strcmp(root->c_str(), payload) == 0);

    std::printf("flatbuffers test OK: %d.%d.%d\n",
                FLATBUFFERS_VERSION_MAJOR,
                FLATBUFFERS_VERSION_MINOR,
                FLATBUFFERS_VERSION_REVISION);
    return 0;
}
