/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opentelemetry: metrics; calls metrics_api::Provider::GetMeterProvider().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/context/context.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/unique_ptr.h>
#include <cassert>
#include <cstdio>
#include <cstdint>

namespace metrics_api = opentelemetry::metrics;
namespace nostd = opentelemetry::nostd;

int main() {
    /* Documented contract: GetMeterProvider() never returns nullptr (default no-op). */
    nostd::shared_ptr<metrics_api::MeterProvider> provider = metrics_api::Provider::GetMeterProvider();
    assert(provider);

    nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("demo-smoke", "1.0");
    assert(meter);

    /* Counter<uint64_t> — sync instrument vtable check (no-op path). */
    nostd::unique_ptr<metrics_api::Counter<uint64_t>> counter =
        meter->CreateUInt64Counter("demo.smoke.counter", "test counter", "1");
    assert(counter);
    counter->Add(1);
    counter->Add(42);

    nostd::unique_ptr<metrics_api::Histogram<double>> hist =
        meter->CreateDoubleHistogram("demo.smoke.histogram", "test histogram", "ms");
    assert(hist);
    hist->Record(3.14, opentelemetry::context::Context{});

    std::printf("opentelemetry metrics test OK: %s\n", OPENTELEMETRY_VERSION);
    return 0;
}
