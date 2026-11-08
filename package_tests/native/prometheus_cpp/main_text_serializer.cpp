/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for prometheus_cpp: text serializer; calls prometheus::BuildCounter().
 */

#include <prometheus/counter.h>
#include <prometheus/registry.h>
#include <prometheus/text_serializer.h>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <string>

int main(void) {
    prometheus::Registry registry;

    auto& family = prometheus::BuildCounter()
                       .Name("demo_smoke_serialized_total")
                       .Help("Demo native-test serializer probe")
                       .Register(registry);
    prometheus::Counter& counter = family.Add({{"path", "ok"}});
    counter.Increment(7.0);

    prometheus::TextSerializer serializer;
    std::ostringstream out;
    serializer.Serialize(out, registry.Collect());
    const std::string text = out.str();

    assert(text.find("# HELP demo_smoke_serialized_total") != std::string::npos);
    assert(text.find("# TYPE demo_smoke_serialized_total counter") !=
           std::string::npos);
    assert(text.find("demo_smoke_serialized_total{path=\"ok\"} 7") !=
           std::string::npos);

    std::printf("prometheus_cpp text_serializer test OK: %zu bytes emitted\n",
                text.size());
    return 0;
}
