/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nlohmann_json: binary; calls json::to_cbor(), json::from_cbor(), json::to_msgpack(), json::from_msgpack().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nlohmann/json.hpp>
#include <cassert>
#include <cstdint>
#include <vector>

using nlohmann::json;

int main() {
    const json original = {
        {"id", 7},
        {"name", "widget"},
        {"count", 64},
        {"ratio", 2.5},
        {"tags", {"red", "green", "blue"}},
        {"active", true},
        {"notes", nullptr},
    };

    /* --- CBOR --- */
    const std::vector<std::uint8_t> cbor = json::to_cbor(original);
    assert(!cbor.empty());
    const json from_cbor = json::from_cbor(cbor);
    assert(from_cbor == original);

    /* --- MessagePack --- */
    const std::vector<std::uint8_t> msgpack = json::to_msgpack(original);
    assert(!msgpack.empty());
    const json from_msgpack = json::from_msgpack(msgpack);
    assert(from_msgpack == original);

    /* --- UBJSON --- */
    const std::vector<std::uint8_t> ubjson = json::to_ubjson(original);
    assert(!ubjson.empty());
    const json from_ubjson = json::from_ubjson(ubjson);
    assert(from_ubjson == original);

    /* The three binary encodings should produce distinct byte streams
     * (i.e. they are different specs, not the same wire format). */
    assert(cbor != msgpack);
    assert(cbor != ubjson);
    assert(msgpack != ubjson);

    /* Truncating a CBOR stream must cause from_cbor to throw (parse error).
     * Catches a broken decoder that silently swallows trailing-byte loss. */
    bool threw = false;
    try {
        std::vector<std::uint8_t> truncated(cbor.begin(), cbor.begin() + cbor.size() / 2);
        (void) json::from_cbor(truncated);
    } catch (const json::exception&) {
        threw = true;
    }
    assert(threw);

    return 0;
}
