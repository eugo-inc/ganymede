/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for prometheus_cpp: core API; calls prometheus::BuildCounter().
 */

#include <prometheus/counter.h>
#include <prometheus/registry.h>
#include <cassert>
#include <cstdio>

int main(void) {
    prometheus::Registry registry;

    auto& counter_family = prometheus::BuildCounter()
                               .Name("demo_smoke_total")
                               .Help("Demo native-test smoke counter")
                               .Register(registry);

    prometheus::Counter& counter = counter_family.Add({{"kind", "smoke"}});
    assert(counter.Value() == 0.0);

    counter.Increment();
    counter.Increment(2.5);
    assert(counter.Value() == 3.5);

    std::printf("prometheus_cpp test OK: counter=%.2f\n", counter.Value());
    return 0;
}
