/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opentelemetry: context; calls nostd::holds_alternative(), nostd::get(), context_api::RuntimeContext::GetCurrent().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/context/context.h>
#include <opentelemetry/context/context_value.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/nostd/variant.h>
#include <cassert>
#include <cstdio>
#include <cstdint>

namespace context_api = opentelemetry::context;
namespace nostd = opentelemetry::nostd;

int main() {
    /* Default-constructed Context is empty. */
    context_api::Context empty{};
    assert(!empty.HasKey("demo-key"));

    /* SetValue returns a NEW Context — persistent semantics. */
    context_api::ContextValue value = static_cast<int64_t>(42);
    context_api::Context with_entry = empty.SetValue("demo-key", value);
    assert(with_entry.HasKey("demo-key"));

    /* Original empty context must NOT see the new entry (immutability). */
    assert(!empty.HasKey("demo-key"));

    /* Retrieve via the ContextValue variant and decode the int64_t arm. */
    context_api::ContextValue retrieved = with_entry.GetValue("demo-key");
    assert(nostd::holds_alternative<int64_t>(retrieved));
    assert(nostd::get<int64_t>(retrieved) == 42);

    /* RuntimeContext singleton must not crash on first access — covers the
     * thread-local stack initialisation path used by Scope/Token attach. */
    context_api::Context current = context_api::RuntimeContext::GetCurrent();
    (void)current;

    std::printf("opentelemetry context test OK: %s\n", OPENTELEMETRY_VERSION);
    return 0;
}
