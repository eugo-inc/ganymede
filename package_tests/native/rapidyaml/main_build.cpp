/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidyaml: build; calls ryml::emitrs_yaml().
 */

#include <ryml.hpp>
#include <c4/yml/std/string.hpp>
#include <cassert>
#include <string>

int main() {
    ryml::Tree tree;
    ryml::NodeRef root = tree.rootref();
    root |= ryml::MAP;

    /* Add a string-valued key. */
    ryml::NodeRef name = root.append_child();
    name.set_key("name");
    name.set_val("demo-build");

    ryml::NodeRef answer = root.append_child();
    answer.set_key("answer");
    answer << 7;

    /* Add a nested SEQ to confirm append_child() works recursively. */
    ryml::NodeRef tags = root.append_child();
    tags.set_key("tags");
    tags |= ryml::SEQ;
    tags.append_child() << "hpc";
    tags.append_child() << "yaml";

    assert(root.is_map());
    assert(root.num_children() == 3);
    assert(tags.is_seq());
    assert(tags.num_children() == 2);

    std::string emitted = ryml::emitrs_yaml<std::string>(tree);
    assert(!emitted.empty());
    assert(emitted.find("name: demo-build") != std::string::npos);
    assert(emitted.find("answer: 7") != std::string::npos);
    assert(emitted.find("hpc") != std::string::npos);
    assert(emitted.find("yaml") != std::string::npos);

    return 0;
}
