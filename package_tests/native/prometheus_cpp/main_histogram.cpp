/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for prometheus_cpp: histogram; calls prometheus::BuildHistogram().
 */

#include <prometheus/client_metric.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <cassert>
#include <cstdio>

int main(void) {
    prometheus::Registry registry;

    auto& family = prometheus::BuildHistogram()
                       .Name("demo_smoke_histogram_seconds")
                       .Help("Demo native-test smoke histogram")
                       .Register(registry);

    prometheus::Histogram::BucketBoundaries buckets{1.0, 5.0, 10.0};
    prometheus::Histogram& hist = family.Add({{"kind", "smoke"}}, buckets);

    /* Observe 4 samples — expected routing:
     *   0.5 → bucket [-inf, 1.0)   (bucket index 0)
     *   3.0 → bucket [1.0,  5.0)   (bucket index 1)
     *   7.0 → bucket [5.0,  10.0)  (bucket index 2)
     *  20.0 → bucket [10.0, +inf)  (bucket index 3, implicit +Inf bucket)
     * sum = 30.5, sample_count = 4
     */
    hist.Observe(0.5);
    hist.Observe(3.0);
    hist.Observe(7.0);
    hist.Observe(20.0);

    prometheus::ClientMetric m = hist.Collect();
    assert(m.histogram.sample_count == 4);
    double diff = m.histogram.sample_sum - 30.5;
    if (diff < 0) diff = -diff;
    assert(diff < 1e-9);

    /* Three explicit boundaries ⇒ four bucket entries: prometheus-cpp's
     * Histogram::Collect() materializes one Counter per explicit boundary PLUS
     * an implicit terminal +Inf bucket, so bucket.size() == boundaries + 1.
     * The bucket counts are CUMULATIVE (each includes all lower buckets). */
    assert(m.histogram.bucket.size() == buckets.size() + 1);
    assert(m.histogram.bucket[0].cumulative_count == 1);  /* 0.5            */
    assert(m.histogram.bucket[1].cumulative_count == 2);  /* +3.0           */
    assert(m.histogram.bucket[2].cumulative_count == 3);  /* +7.0           */
    assert(m.histogram.bucket[3].cumulative_count == 4);  /* +20.0 → +Inf   */

    std::printf("prometheus_cpp histogram test OK: count=%llu sum=%.2f\n",
                static_cast<unsigned long long>(m.histogram.sample_count),
                m.histogram.sample_sum);
    return 0;
}
