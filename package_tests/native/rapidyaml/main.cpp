/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidyaml: core API; calls ryml::parse_in_arena(), ryml::to_csubstr(), ryml::emitrs_yaml().
 */

#include <ryml.hpp>
#include <c4/yml/std/string.hpp>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    /* A small YAML document covering a map with a string and an int leaf. */
    const std::string yaml_src =
        "name: demo\n"
        "answer: 42\n";

    /* Parse into the arena (ryml copies the source into its own storage). */
    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_src));

    ryml::ConstNodeRef root = tree.rootref();
    assert(root.is_map());
    assert(root.num_children() == 2);

    /* String leaf — compare the scalar substring directly. */
    ryml::ConstNodeRef name_node = root["name"];
    assert(name_node.has_val());
    assert(name_node.val() == "demo");

    int answer = 0;
    root["answer"] >> answer;
    assert(answer == 42);

    std::string emitted = ryml::emitrs_yaml<std::string>(tree);
    assert(!emitted.empty());
    assert(emitted.find("name: demo") != std::string::npos);
    assert(emitted.find("answer: 42") != std::string::npos);

    std::cout << "rapidyaml test OK" << std::endl;
    return 0;
}
