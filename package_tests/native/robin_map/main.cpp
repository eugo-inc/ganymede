/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for robin_map: core API; compiles against <tsl/robin_map.h>, <tsl/robin_set.h>.
 */

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#include <cassert>
#include <string>
#include <iostream>

int main() {
    /* Primary type: robin_map<int, std::string>. */
    tsl::robin_map<int, std::string> map;
    map.insert({1, "one"});
    map.insert({2, "two"});
    map[3] = "three";

    assert(map.size() == 3);
    assert(map.at(1) == "one");
    assert(map.at(2) == "two");
    assert(map.at(3) == "three");
    assert(map.find(99) == map.end());

    assert(map.erase(2) == 1);
    assert(map.size() == 2);
    assert(map.find(2) == map.end());

    /* Sibling type: robin_set<int>. */
    tsl::robin_set<int> set;
    set.insert(10);
    set.insert(20);
    set.insert(10);  /* duplicate */
    assert(set.size() == 2);
    assert(set.count(10) == 1);
    assert(set.count(30) == 0);

    std::cout << "robin_map test OK" << std::endl;
    return 0;
}
