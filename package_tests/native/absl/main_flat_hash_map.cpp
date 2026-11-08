/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: flat hash map; compiles against <absl/container/flat_hash_map.h>.
 */

#include <absl/container/flat_hash_map.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    absl::flat_hash_map<std::string, int> m;
    assert(m.empty());
    assert(m.size() == 0);

    /* Insert distinct keys. */
    m.insert({"alpha", 1});
    m.insert({"beta", 2});
    m.insert({"gamma", 3});
    assert(m.size() == 3);
    assert(m.contains("alpha"));
    assert(m.contains("beta"));
    assert(m.contains("gamma"));
    assert(!m.contains("delta"));
    assert(m.count("beta") == 1);
    assert(m.count("delta") == 0);

    /* Lookup must return the inserted values. */
    auto it = m.find("beta");
    assert(it != m.end());
    assert(it->second == 2);

    int& delta_ref = m["delta"];
    assert(delta_ref == 0);
    assert(m.size() == 4);
    assert(m.contains("delta"));
    delta_ref = 99;
    assert(m["delta"] == 99);

    /* erase by key must remove and return 1; second erase must return 0. */
    assert(m.erase("alpha") == 1);
    assert(m.size() == 3);
    assert(!m.contains("alpha"));
    assert(m.erase("alpha") == 0);

    /* Iterate and sum remaining values to confirm contents survived rehash. */
    int sum = 0;
    for (const auto& [k, v] : m) {
        (void)k;
        sum += v;
    }
    /* beta(2) + gamma(3) + delta(99) = 104 */
    assert(sum == 104);

    std::printf("absl flat_hash_map test OK: size=%zu sum=%d\n", m.size(), sum);
    return 0;
}
