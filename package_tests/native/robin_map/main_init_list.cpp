/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for robin_map: init list; compiles against <tsl/robin_map.h>, <tsl/robin_set.h>.
 */

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#include <cassert>
#include <string>
#include <utility>
#include <vector>

int main() {
    /* (1) initializer_list constructor for robin_map. */
    tsl::robin_map<int, std::string> m{
        {1, "one"},
        {2, "two"},
        {3, "three"},
    };
    assert(m.size() == 3);
    assert(m.at(1) == "one");
    assert(m.at(2) == "two");
    assert(m.at(3) == "three");

    /* (2) Duplicates in an initializer list — first-wins, size collapses. */
    tsl::robin_map<int, std::string> dup{
        {1, "first"},
        {2, "two"},
        {1, "second"},  /* duplicate key */
    };
    assert(dup.size() == 2);
    /* tsl::robin_map (like std::unordered_map) keeps the FIRST inserted value. */
    assert(dup.at(1) == "first");

    /* (3) initializer_list constructor for robin_set + duplicate collapse. */
    tsl::robin_set<int> s{1, 2, 3, 2, 1};
    assert(s.size() == 3);
    assert(s.count(1) == 1);
    assert(s.count(2) == 1);
    assert(s.count(3) == 1);
    assert(s.count(99) == 0);

    /* (4) Iterator-range constructor — copy from a std::vector of pairs. */
    std::vector<std::pair<int, std::string>> src{
        {10, "ten"},
        {20, "twenty"},
        {30, "thirty"},
    };
    tsl::robin_map<int, std::string> from_range(src.begin(), src.end());
    assert(from_range.size() == 3);
    assert(from_range.at(10) == "ten");
    assert(from_range.at(20) == "twenty");
    assert(from_range.at(30) == "thirty");

    /* (5) Bulk insert via insert(first, last) into a NON-empty map. Existing
     *     keys keep their existing value (insert is "if-not-present"), new
     *     keys get added. */
    tsl::robin_map<int, std::string> target;
    target.insert({10, "ORIGINAL"});
    target.insert({40, "forty"});
    target.insert(src.begin(), src.end());
    /* {10, ORIGINAL} kept; {20, twenty} + {30, thirty} added; {40, forty} kept. */
    assert(target.size() == 4);
    assert(target.at(10) == "ORIGINAL");
    assert(target.at(20) == "twenty");
    assert(target.at(30) == "thirty");
    assert(target.at(40) == "forty");

    /* (6) Bulk insert via initializer_list overload. */
    tsl::robin_map<int, std::string> il_target;
    il_target.insert({{100, "hundred"}, {200, "two hundred"}});
    assert(il_target.size() == 2);
    assert(il_target.at(100) == "hundred");
    assert(il_target.at(200) == "two hundred");

    /* (7) emplace — constructs the (key, value) pair in place; returns
     *     pair<iterator, bool> where bool is true iff the key was new. */
    tsl::robin_map<int, std::string> em;
    auto r1 = em.emplace(1, "one");
    assert(r1.second == true);
    assert(r1.first->first == 1);
    assert(r1.first->second == "one");
    auto r2 = em.emplace(1, "duplicate");
    assert(r2.second == false);            /* not inserted */
    assert(r2.first->second == "one");     /* still the original */
    assert(em.size() == 1);

    return 0;
}
