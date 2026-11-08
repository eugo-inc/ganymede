/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidyaml: anchors; calls ryml::parse_in_arena(), ryml::to_csubstr().
 */

#include <ryml.hpp>
#include <c4/yml/std/string.hpp>
#include <cassert>
#include <string>

int main() {
    /* Two map values share the same anchor — *addr_ref expands to a copy of
     * the addr map. */
    const std::string yaml_src =
        "addr: &addr\n"
        "  city: Berkeley\n"
        "  state: CA\n"
        "billing: *addr\n"
        "shipping: *addr\n";

    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_src));
    ryml::ConstNodeRef root = tree.rootref();
    assert(root.is_map());
    assert(root.num_children() == 3);

    /* Pre-resolve: the anchor on "addr" is recorded. */
    ryml::ConstNodeRef addr = root["addr"];
    assert(addr.has_val_anchor());
    assert(addr.val_anchor() == "addr");

    /* Pre-resolve: "billing" and "shipping" are val-refs pointing at "addr". */
    ryml::ConstNodeRef billing = root["billing"];
    ryml::ConstNodeRef shipping = root["shipping"];
    assert(billing.is_val_ref());
    assert(shipping.is_val_ref());
    assert(billing.val_ref() == "addr");
    assert(shipping.val_ref() == "addr");

    /* Resolve. After this, aliases are substituted in place and the metadata
     * is cleared. */
    tree.resolve();

    /* Re-acquire refs — the tree may have been restructured. */
    ryml::ConstNodeRef root2 = tree.rootref();
    ryml::ConstNodeRef addr2 = root2["addr"];
    ryml::ConstNodeRef billing2 = root2["billing"];
    ryml::ConstNodeRef shipping2 = root2["shipping"];

    /* Post-resolve: anchor metadata is gone. */
    assert(!addr2.has_val_anchor());

    /* Post-resolve: every alias is now a full MAP with the anchor's contents. */
    assert(billing2.is_map());
    assert(billing2.num_children() == 2);
    assert(billing2["city"].val() == "Berkeley");
    assert(billing2["state"].val() == "CA");

    assert(shipping2.is_map());
    assert(shipping2.num_children() == 2);
    assert(shipping2["city"].val() == "Berkeley");
    assert(shipping2["state"].val() == "CA");

    /* Post-resolve: nothing claims to be a val-ref anymore. */
    assert(!billing2.is_val_ref());
    assert(!shipping2.is_val_ref());

    return 0;
}
