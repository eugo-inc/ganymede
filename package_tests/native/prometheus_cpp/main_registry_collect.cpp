/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for prometheus_cpp: registry collect; calls prometheus::BuildCounter(), prometheus::BuildGauge().
 */

#include <prometheus/client_metric.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/metric_family.h>
#include <prometheus/metric_type.h>
#include <prometheus/registry.h>
#include <cassert>
#include <cstdio>
#include <vector>

int main(void) {
    prometheus::Registry registry;

    auto& counter_family = prometheus::BuildCounter()
                               .Name("demo_smoke_collect_counter_total")
                               .Help("Demo collect probe counter")
                               .Register(registry);
    counter_family.Add({{"slot", "0"}}).Increment(2.0);
    counter_family.Add({{"slot", "1"}}).Increment(5.0);

    auto& gauge_family = prometheus::BuildGauge()
                             .Name("demo_smoke_collect_gauge")
                             .Help("Demo collect probe gauge")
                             .Register(registry);
    gauge_family.Add({{"slot", "only"}}).Set(11.0);

    std::vector<prometheus::MetricFamily> mfs = registry.Collect();
    assert(mfs.size() == 2);

    /* Order of MetricFamily entries in Registry::Collect() is not guaranteed
     * to match registration order — discover each by name. */
    const prometheus::MetricFamily* counter_mf = nullptr;
    const prometheus::MetricFamily* gauge_mf = nullptr;
    for (const auto& mf : mfs) {
        if (mf.name == "demo_smoke_collect_counter_total") counter_mf = &mf;
        else if (mf.name == "demo_smoke_collect_gauge") gauge_mf = &mf;
    }
    assert(counter_mf != nullptr);
    assert(gauge_mf != nullptr);

    /* Counter family contract: tagged as Counter, two dimensional samples,
     * each carrying the value its Counter was Incremented to. */
    assert(counter_mf->type == prometheus::MetricType::Counter);
    assert(counter_mf->help == "Demo collect probe counter");
    assert(counter_mf->metric.size() == 2);

    double counter_sum = 0.0;
    for (const auto& cm : counter_mf->metric) {
        /* Counter samples populate the .counter field, not .gauge. */
        counter_sum += cm.counter.value;
        assert(cm.label.size() == 1);
        assert(cm.label[0].name == "slot");
    }
    assert(counter_sum == 7.0);  /* 2.0 + 5.0 */

    /* Gauge family contract: tagged as Gauge, one dimensional sample at 11.0. */
    assert(gauge_mf->type == prometheus::MetricType::Gauge);
    assert(gauge_mf->help == "Demo collect probe gauge");
    assert(gauge_mf->metric.size() == 1);
    assert(gauge_mf->metric[0].gauge.value == 11.0);
    assert(gauge_mf->metric[0].label.size() == 1);
    assert(gauge_mf->metric[0].label[0].name == "slot");
    assert(gauge_mf->metric[0].label[0].value == "only");

    std::printf("prometheus_cpp registry_collect test OK: families=%zu "
                "counter_samples=%zu gauge_samples=%zu\n",
                mfs.size(), counter_mf->metric.size(),
                gauge_mf->metric.size());
    return 0;
}
