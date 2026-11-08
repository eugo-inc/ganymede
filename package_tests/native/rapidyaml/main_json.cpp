/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidyaml: json; calls ryml::parse_json_in_arena(), ryml::to_csubstr(), ryml::emitrs_json().
 */

#include <ryml.hpp>
#include <c4/yml/std/string.hpp>
#include <cassert>
#include <string>

int main() {
    /* A JSON document — strictly valid JSON, not YAML. */
    const std::string json_src =
        "{\"name\":\"demo\",\"answer\":42,\"tags\":[\"hpc\",\"yaml\"]}";

    /* Use the dedicated JSON parser path, not parse_in_arena. */
    ryml::Tree tree = ryml::parse_json_in_arena(ryml::to_csubstr(json_src));

    ryml::ConstNodeRef root = tree.rootref();
    assert(root.is_map());
    assert(root.num_children() == 3);

    /* String key — confirm scalar matches. */
    assert(root["name"].val() == "demo");

    int answer = 0;
    root["answer"] >> answer;
    assert(answer == 42);

    /* Array under the "tags" key — confirms JSON arrays land as SEQ nodes. */
    ryml::ConstNodeRef tags = root["tags"];
    assert(tags.is_seq());
    assert(tags.num_children() == 2);
    assert(tags[0].val() == "hpc");
    assert(tags[1].val() == "yaml");

    /* Emit back to JSON via the dedicated JSON emitter. The output must
     * contain JSON-style quoted keys, not YAML-style bare keys. */
    std::string emitted = ryml::emitrs_json<std::string>(tree);
    assert(!emitted.empty());
    assert(emitted.find("\"name\"") != std::string::npos);
    assert(emitted.find("\"demo\"") != std::string::npos);
    assert(emitted.find("\"answer\"") != std::string::npos);
    assert(emitted.find("42") != std::string::npos);
    assert(emitted.find("\"tags\"") != std::string::npos);
    assert(emitted.find("\"hpc\"") != std::string::npos);

    assert(emitted.find("- ") == std::string::npos);

    return 0;
}
