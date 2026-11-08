/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nlohmann_json: core API; calls json::parse().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nlohmann/json.hpp>
#include <cassert>
#include <string>
#include <iostream>

using nlohmann::json;

int main() {
    json original = {
        {"answer", 42},
        {"name", "demo"},
        {"flags", {1, 2, 3}},
        {"nested", {{"ok", true}}},
    };

    const std::string serialized = original.dump();
    assert(!serialized.empty());

    const json parsed = json::parse(serialized);
    assert(parsed == original);

    /* Typed access — exercises the SFINAE-gated get<T>() templates. */
    assert(parsed["answer"].get<int>() == 42);
    assert(parsed["name"].get<std::string>() == "demo");
    assert(parsed["flags"].size() == 3);
    assert(parsed["flags"][2].get<int>() == 3);
    assert(parsed["nested"]["ok"].get<bool>() == true);

    std::cout << "nlohmann_json test OK: "
              << NLOHMANN_JSON_VERSION_MAJOR << "."
              << NLOHMANN_JSON_VERSION_MINOR << "."
              << NLOHMANN_JSON_VERSION_PATCH << std::endl;
    return 0;
}
