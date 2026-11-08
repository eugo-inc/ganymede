/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opentelemetry: baggage; calls baggage_api::Baggage::GetDefault(), baggage_api::Baggage::FromHeader().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/baggage/baggage.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <cassert>
#include <cstdio>
#include <string>

namespace baggage_api = opentelemetry::baggage;
namespace nostd = opentelemetry::nostd;

int main() {
    /* Default baggage must be non-null per documented contract. */
    nostd::shared_ptr<baggage_api::Baggage> empty = baggage_api::Baggage::GetDefault();
    assert(empty);

    /* Set returns a NEW Baggage; empty must stay empty (persistent semantics). */
    nostd::shared_ptr<baggage_api::Baggage> with_entry = empty->Set("demo-key", "demo-value");
    assert(with_entry);
    assert(with_entry.get() != empty.get());

    /* New instance carries the entry. */
    std::string out_value;
    bool found = with_entry->GetValue("demo-key", out_value);
    assert(found);
    assert(out_value == "demo-value");

    /* Original empty instance does NOT see the entry. */
    std::string ignored;
    bool found_in_empty = empty->GetValue("demo-key", ignored);
    assert(!found_in_empty);

    std::string header = with_entry->ToHeader();
    assert(!header.empty());
    nostd::shared_ptr<baggage_api::Baggage> parsed = baggage_api::Baggage::FromHeader(header);
    assert(parsed);
    std::string parsed_value;
    bool parsed_found = parsed->GetValue("demo-key", parsed_value);
    assert(parsed_found);
    assert(parsed_value == "demo-value");

    std::printf("opentelemetry baggage test OK: header=%s\n", header.c_str());
    return 0;
}
