/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidyaml: seq; calls ryml::parse_in_arena(), ryml::to_csubstr().
 */

#include <ryml.hpp>
#include <c4/yml/std/string.hpp>
#include <cassert>
#include <cstddef>
#include <string>

int main() {
    const std::string yaml_src =
        "project: rapidyaml\n"
        "items:\n"
        "  - alpha\n"
        "  - beta\n"
        "  - gamma\n"
        "nested:\n"
        "  inner:\n"
        "    - 1\n"
        "    - 2\n";

    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_src));
    ryml::ConstNodeRef root = tree.rootref();

    assert(root.is_map());
    assert(root.num_children() == 3);

    /* The "items" key is a SEQ with three string scalars. */
    ryml::ConstNodeRef items = root["items"];
    assert(items.is_seq());
    assert(items.num_children() == 3);
    assert(items[0].val() == "alpha");
    assert(items[1].val() == "beta");
    assert(items[2].val() == "gamma");

    std::size_t walked = 0;
    for (ryml::ConstNodeRef c : items.children()) {
        (void)c;
        ++walked;
    }
    assert(walked == 3);

    /* Nested MAP -> SEQ of ints. Confirms recursive structure + typed read on a
     * child of a child. */
    ryml::ConstNodeRef nested = root["nested"];
    assert(nested.is_map());
    ryml::ConstNodeRef inner = nested["inner"];
    assert(inner.is_seq());
    assert(inner.num_children() == 2);

    int first = 0, second = 0;
    inner[0] >> first;
    inner[1] >> second;
    assert(first == 1);
    assert(second == 2);

    return 0;
}
