/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for robin_map: capacity; compiles against <tsl/robin_map.h>.
 */

#include <tsl/robin_map.h>
#include <cassert>

int main() {
    tsl::robin_map<int, int> m;

    /* Empty map invariants. */
    assert(m.size() == 0);
    assert(m.empty());
    assert(m.load_factor() == 0.0f);

    /* reserve(n) guarantees no rehash for the next n insertions and forces
     * bucket_count() to grow to at least the required size. */
    m.reserve(128);
    const auto bc_after_reserve = m.bucket_count();
    assert(bc_after_reserve >= 128);

    /* Bulk insert; bucket_count must NOT shrink. */
    for (int i = 0; i < 100; ++i) {
        m.insert({i, i * 10});
    }
    assert(m.size() == 100);
    assert(m.bucket_count() >= bc_after_reserve);

    /* load_factor = size / bucket_count, strictly in (0, max_load_factor]. */
    const float lf = m.load_factor();
    const float mlf = m.max_load_factor();
    assert(lf > 0.0f);
    assert(lf <= mlf);
    assert(mlf > 0.0f && mlf <= 1.0f);

    /* All inserted keys still resolvable after any internal rehashing. */
    for (int i = 0; i < 100; ++i) {
        auto it = m.find(i);
        assert(it != m.end());
        assert(it->second == i * 10);
    }

    /* rehash(n) sets bucket_count to at least n; data must survive. */
    m.rehash(1024);
    assert(m.bucket_count() >= 1024);
    assert(m.size() == 100);
    assert(m.at(42) == 420);

    m.max_load_factor(0.5f);
    assert(m.max_load_factor() == 0.5f);

    return 0;
}
