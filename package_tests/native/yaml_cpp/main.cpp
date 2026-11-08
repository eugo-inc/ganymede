/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yaml_cpp: core API; calls YAML::Load().
 */

#include <yaml-cpp/yaml.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* Parse a small document — exercises scalar, sequence, and map nodes. */
    YAML::Node doc = YAML::Load("name: demo\nvalues: [1, 2, 3]\n");
    assert(doc.IsMap());
    assert(doc["name"].as<std::string>() == "demo");
    assert(doc["values"].IsSequence());
    assert(doc["values"].size() == 3);
    assert(doc["values"][2].as<int>() == 3);

    YAML::Emitter out;
    out << doc;
    assert(out.good());
    std::string emitted = out.c_str();
    assert(!emitted.empty());

    std::printf("yaml_cpp test OK: parsed %zu top-level keys\n", doc.size());
    return 0;
}
