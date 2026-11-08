/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for robin_map: copy move; compiles against <tsl/robin_map.h>.
 */

#include <tsl/robin_map.h>
#include <cassert>
#include <string>
#include <utility>

int main() {
    tsl::robin_map<int, std::string> src;
    src.insert({1, "one"});
    src.insert({2, "two"});
    src.insert({3, "three"});
    assert(src.size() == 3);

    /* (1) Copy construction — both maps independently own their data. */
    tsl::robin_map<int, std::string> copy(src);
    assert(copy.size() == 3);
    assert(copy.at(1) == "one");
    assert(copy.at(2) == "two");
    assert(copy.at(3) == "three");

    /* Mutating the copy must NOT affect the source. */
    copy[1] = "ONE";
    copy.erase(2);
    assert(copy.size() == 2);
    assert(copy.at(1) == "ONE");
    /* src must be untouched. */
    assert(src.size() == 3);
    assert(src.at(1) == "one");
    assert(src.at(2) == "two");

    /* (2) Copy assignment — replaces previous contents. */
    tsl::robin_map<int, std::string> assigned;
    assigned.insert({99, "ninety-nine"});
    assigned = src;
    assert(assigned.size() == 3);
    assert(assigned.at(1) == "one");
    assert(assigned.find(99) == assigned.end());

    /* (3) Move construction — source ends up in a valid-but-unspecified state;
     *     the destination owns the data. */
    tsl::robin_map<int, std::string> donor;
    donor.insert({10, "ten"});
    donor.insert({20, "twenty"});
    tsl::robin_map<int, std::string> moved(std::move(donor));
    assert(moved.size() == 2);
    assert(moved.at(10) == "ten");
    assert(moved.at(20) == "twenty");
    donor.clear();
    assert(donor.empty());

    /* (4) Move assignment — destination's prior contents replaced. */
    tsl::robin_map<int, std::string> mv_target;
    mv_target.insert({77, "seventy-seven"});
    tsl::robin_map<int, std::string> mv_src;
    mv_src.insert({100, "hundred"});
    mv_src.insert({200, "two hundred"});
    mv_target = std::move(mv_src);
    assert(mv_target.size() == 2);
    assert(mv_target.at(100) == "hundred");
    assert(mv_target.at(200) == "two hundred");
    assert(mv_target.find(77) == mv_target.end());

    /* (5) swap() — must exchange contents in O(1) without invalidating ownership. */
    tsl::robin_map<int, std::string> a;
    a.insert({1, "a1"});
    tsl::robin_map<int, std::string> b;
    b.insert({2, "b2"});
    b.insert({3, "b3"});
    a.swap(b);
    assert(a.size() == 2);
    assert(b.size() == 1);
    assert(a.at(2) == "b2");
    assert(a.at(3) == "b3");
    assert(b.at(1) == "a1");

    tsl::robin_map<int, std::string> e1;
    e1.insert({1, "x"});
    e1.insert({2, "y"});
    tsl::robin_map<int, std::string> e2;
    e2.insert({2, "y"});
    e2.insert({1, "x"});
    assert(e1 == e2);
    e2[1] = "different";
    assert(e1 != e2);

    return 0;
}
