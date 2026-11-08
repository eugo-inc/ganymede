/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for prometheus_cpp: summary; calls prometheus::BuildSummary().
 */

#include <prometheus/client_metric.h>
#include <prometheus/registry.h>
#include <prometheus/summary.h>
#include <cassert>
#include <cstdio>

int main(void) {
    prometheus::Registry registry;

    auto& family = prometheus::BuildSummary()
                       .Name("demo_smoke_summary_seconds")
                       .Help("Demo native-test smoke summary")
                       .Register(registry);

    /* Quantile{phi, tolerated_error}. */
    prometheus::Summary::Quantiles quantiles{
        {0.5,  0.05},
        {0.9,  0.01},
        {0.99, 0.001},
    };
    prometheus::Summary& summary = family.Add({{"kind", "smoke"}}, quantiles);

    double expected_sum = 0.0;
    for (int i = 1; i <= 100; ++i) {
        const double v = static_cast<double>(i);
        summary.Observe(v);
        expected_sum += v;
    }

    prometheus::ClientMetric m = summary.Collect();
    assert(m.summary.sample_count == 100);

    double diff = m.summary.sample_sum - expected_sum;
    if (diff < 0) diff = -diff;
    assert(diff < 1e-9);

    assert(m.summary.quantile.size() == quantiles.size());

    for (const auto& q : m.summary.quantile) {
        assert(q.value >= 1.0);
        assert(q.value <= 100.0);
    }

    assert(m.summary.quantile[0].quantile == 0.5);
    assert(m.summary.quantile[1].quantile == 0.9);
    assert(m.summary.quantile[2].quantile == 0.99);

    std::printf("prometheus_cpp summary test OK: count=%llu sum=%.2f "
                "p50=%.2f p90=%.2f p99=%.2f\n",
                static_cast<unsigned long long>(m.summary.sample_count),
                m.summary.sample_sum,
                m.summary.quantile[0].value,
                m.summary.quantile[1].value,
                m.summary.quantile[2].value);
    return 0;
}
