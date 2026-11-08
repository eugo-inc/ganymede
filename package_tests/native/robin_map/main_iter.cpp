/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for robin_map: iter; compiles against <tsl/robin_map.h>.
 */

#include <tsl/robin_map.h>
#include <cassert>
#include <string>

int main() {
    tsl::robin_map<int, std::string> m;
    m.insert({1, "one"});
    m.insert({2, "two"});
    m.insert({3, "three"});
    m.insert({4, "four"});
    assert(m.size() == 4);

    /* (1) Range-based for: visit every entry exactly once. */
    int key_sum = 0;
    std::size_t visited = 0;
    for (const auto& kv : m) {
        key_sum += kv.first;
        ++visited;
    }
    assert(visited == 4);
    assert(key_sum == 1 + 2 + 3 + 4);

    /* (2) find() returns an iterator; equality with end() distinguishes hit/miss. */
    auto it = m.find(3);
    assert(it != m.end());
    assert(it->first == 3);
    assert(it->second == "three");
    assert(m.find(99) == m.end());

    /* (3) erase(it) returns an iterator to the next live element; map shrinks. */
    auto next = m.erase(it);
    (void)next; /* iteration order is unspecified; just exercise the call. */
    assert(m.size() == 3);
    assert(m.find(3) == m.end());

    /* (4) tsl-specific: mutable_iterator() — mutate the mapped value via a
     *     const-key iterator. The pair returned is (key_const, value_mutable). */
    auto cit = m.find(1);
    assert(cit != m.end());
    auto mit = m.mutable_iterator(cit);
    mit.value() = "ONE";
    assert(m.at(1) == "ONE");

    /* (5) Whole-map clear leaves it empty + iterator range degenerate. */
    m.clear();
    assert(m.empty());
    assert(m.begin() == m.end());

    return 0;
}
