/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for opentelemetry: core API; calls trace_api::Provider::GetTracerProvider().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <cassert>
#include <cstdio>

namespace trace_api = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

int main() {
#ifndef OPENTELEMETRY_VERSION
#error "opentelemetry/version.h did not define OPENTELEMETRY_VERSION — broken install."
#endif
    assert(OPENTELEMETRY_VERSION_MAJOR >= 1);

    /* Documented contract: GetTracerProvider() never returns nullptr (default: no-op). */
    nostd::shared_ptr<trace_api::TracerProvider> provider = trace_api::Provider::GetTracerProvider();
    assert(provider);

    nostd::shared_ptr<trace_api::Tracer> tracer = provider->GetTracer("demo-smoke", "1.0");
    assert(tracer);
    nostd::shared_ptr<trace_api::Span> span = tracer->StartSpan("demo-smoke-span");
    assert(span);
    span->End();

    std::printf("opentelemetry test OK: %s\n", OPENTELEMETRY_VERSION);
    return 0;
}
