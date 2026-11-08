/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yaml_cpp: iterators; calls YAML::Load().
 */

#include <yaml-cpp/yaml.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

int main() {
    YAML::Node doc = YAML::Load(
        "fruits:\n"
        "  - apple\n"
        "  - banana\n"
        "  - cherry\n"
        "scores:\n"
        "  alice: 90\n"
        "  bob: 85\n"
        "  carol: 78\n");

    /* Iterate a sequence — verify count + ordered values. */
    YAML::Node fruits = doc["fruits"];
    assert(fruits.IsSequence());
    std::vector<std::string> collected;
    for (YAML::const_iterator it = fruits.begin(); it != fruits.end(); ++it) {
        collected.push_back(it->as<std::string>());
    }
    assert(collected.size() == 3);
    assert(collected[0] == "apple");
    assert(collected[1] == "banana");
    assert(collected[2] == "cherry");

    /* Iterate a map — verify key/value access via first/second. */
    YAML::Node scores = doc["scores"];
    assert(scores.IsMap());
    int total = 0;
    int count = 0;
    for (YAML::const_iterator it = scores.begin(); it != scores.end(); ++it) {
        std::string key = it->first.as<std::string>();
        int val = it->second.as<int>();
        assert(!key.empty());
        assert(val > 0);
        total += val;
        ++count;
    }
    assert(count == 3);
    assert(total == 90 + 85 + 78);

    std::printf("yaml_cpp iterators test OK: %d map entries summing to %d\n",
                count, total);
    return 0;
}
