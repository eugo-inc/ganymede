/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for prometheus_cpp: family labels; calls prometheus::BuildCounter().
 */

#include <prometheus/counter.h>
#include <prometheus/family.h>
#include <prometheus/registry.h>
#include <cassert>
#include <cstdio>

int main(void) {
    prometheus::Registry registry;

    auto& family = prometheus::BuildCounter()
                       .Name("demo_smoke_requests_total")
                       .Help("Demo native-test multi-label counter family")
                       .Register(registry);

    const prometheus::Labels labels_a{{"method", "GET"}, {"path", "/a"}};
    const prometheus::Labels labels_b{{"method", "POST"}, {"path", "/b"}};

    /* Initial state: neither label-set has been added yet. */
    assert(!family.Has(labels_a));
    assert(!family.Has(labels_b));

    prometheus::Counter& counter_a = family.Add(labels_a);
    prometheus::Counter& counter_b = family.Add(labels_b);
    assert(family.Has(labels_a));
    assert(family.Has(labels_b));

    /* Distinct label-sets must hand back distinct Counter instances. */
    assert(&counter_a != &counter_b);

    counter_a.Increment(3.0);
    counter_b.Increment(7.0);
    assert(counter_a.Value() == 3.0);
    assert(counter_b.Value() == 7.0);

    /* Idempotent Add: same label-set returns the same Counter — must NOT
     * reset its value. This is the dedup-on-hash contract. */
    prometheus::Counter& counter_a_again = family.Add(labels_a);
    assert(&counter_a_again == &counter_a);
    assert(counter_a_again.Value() == 3.0);

    /* Remove drops the dimensional data without disturbing siblings. */
    family.Remove(&counter_a);
    assert(!family.Has(labels_a));
    assert(family.Has(labels_b));
    assert(counter_b.Value() == 7.0);

    /* Remove of an unknown pointer is a documented no-op (must not crash). */
    family.Remove(&counter_a);
    assert(!family.Has(labels_a));

    /* Re-Add after Remove creates a fresh Counter (zeroed). */
    prometheus::Counter& counter_a_fresh = family.Add(labels_a);
    assert(family.Has(labels_a));
    assert(counter_a_fresh.Value() == 0.0);

    std::printf("prometheus_cpp family/labels test OK: a_fresh=%.2f b=%.2f\n",
                counter_a_fresh.Value(), counter_b.Value());
    return 0;
}
