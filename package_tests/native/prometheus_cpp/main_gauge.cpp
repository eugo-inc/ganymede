/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for prometheus_cpp: gauge; calls prometheus::BuildGauge().
 */

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <cassert>
#include <cstdio>

int main(void) {
    prometheus::Registry registry;

    auto& gauge_family = prometheus::BuildGauge()
                             .Name("demo_smoke_gauge")
                             .Help("Demo native-test smoke gauge")
                             .Register(registry);

    prometheus::Gauge& gauge = gauge_family.Add({{"kind", "smoke"}});
    assert(gauge.Value() == 0.0);

    gauge.Set(42.0);
    assert(gauge.Value() == 42.0);

    gauge.Increment(8.0);
    assert(gauge.Value() == 50.0);

    gauge.Decrement(20.0);
    assert(gauge.Value() == 30.0);

    gauge.Decrement();
    assert(gauge.Value() == 29.0);

    std::printf("prometheus_cpp gauge test OK: gauge=%.2f\n", gauge.Value());
    return 0;
}
