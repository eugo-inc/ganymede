/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for flatbuffers: flexbuffers; calls flexbuffers::GetRoot().
 */

#include <flatbuffers/flexbuffers.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>

int main() {
    flexbuffers::Builder fbb;

    fbb.Map([&]() {
        fbb.Int("hp", 42);
        fbb.Float("ratio", 1.5f);
        fbb.String("name", "demo-flex-probe");
        fbb.Bool("alive", true);
        fbb.Vector("xs", [&]() {
            fbb.Int(10);
            fbb.Int(20);
            fbb.Int(30);
        });
    });
    fbb.Finish();

    /* Buffer must be non-empty. */
    const std::vector<uint8_t> &buf = fbb.GetBuffer();
    assert(!buf.empty());

    /* Deserialize: root must be a Map. */
    flexbuffers::Reference root = flexbuffers::GetRoot(buf);
    assert(root.IsMap() && "flexbuffers root is not a Map");

    flexbuffers::Map m = root.AsMap();
    assert(m.size() == 5);

    /* Typed reads — each catches a different breakage mode of the Reference
     * accessor dispatch table. */
    assert(m["hp"].AsInt64() == 42);
    assert(m["ratio"].AsFloat() > 1.49f && m["ratio"].AsFloat() < 1.51f);
    assert(std::strcmp(m["name"].AsString().c_str(), "demo-flex-probe") == 0);
    assert(m["alive"].AsBool() == true);

    /* Nested vector access. */
    flexbuffers::Reference xs_ref = m["xs"];
    assert(xs_ref.IsVector() && "expected nested vector");
    flexbuffers::Vector xs = xs_ref.AsVector();
    assert(xs.size() == 3);
    assert(xs[0].AsInt64() == 10);
    assert(xs[1].AsInt64() == 20);
    assert(xs[2].AsInt64() == 30);

    std::printf("flatbuffers flexbuffers test OK: %zu map keys, %zu vector elts\n",
                m.size(), xs.size());
    return 0;
}
