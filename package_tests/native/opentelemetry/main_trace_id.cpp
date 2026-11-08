/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for opentelemetry: trace id; compiles against <opentelemetry/version.h>, <opentelemetry/trace/trace_id.h>, <opentelemetry/trace/span_id.h>.
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/trace/trace_id.h>
#include <opentelemetry/trace/span_id.h>
#include <opentelemetry/nostd/span.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>

namespace trace_api = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

int main() {
    /* Default-constructed identifiers are the documented "invalid" sentinel. */
    trace_api::TraceId default_trace{};
    trace_api::SpanId default_span{};
    assert(!default_trace.IsValid());
    assert(!default_span.IsValid());

    const uint8_t trace_bytes[16] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    };
    const uint8_t span_bytes[8] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    };

    trace_api::TraceId trace_id{nostd::span<const uint8_t, 16>{trace_bytes}};
    trace_api::SpanId span_id{nostd::span<const uint8_t, 8>{span_bytes}};
    assert(trace_id.IsValid());
    assert(span_id.IsValid());

    uint8_t trace_out[16] = {0};
    uint8_t span_out[8] = {0};
    trace_id.CopyBytesTo(nostd::span<uint8_t, 16>{trace_out});
    span_id.CopyBytesTo(nostd::span<uint8_t, 8>{span_out});
    assert(std::memcmp(trace_out, trace_bytes, 16) == 0);
    assert(std::memcmp(span_out, span_bytes, 8) == 0);

    /* Lower-case base16 encoding, as used in W3C traceparent headers. */
    char trace_hex[32];
    char span_hex[16];
    trace_id.ToLowerBase16(nostd::span<char, 32>{trace_hex});
    span_id.ToLowerBase16(nostd::span<char, 16>{span_hex});
    const std::string expected_trace_hex = "0102030405060708090a0b0c0d0e0f10";
    const std::string expected_span_hex = "1122334455667788";
    assert(std::string(trace_hex, 32) == expected_trace_hex);
    assert(std::string(span_hex, 16) == expected_span_hex);

    trace_api::TraceId trace_id_copy{nostd::span<const uint8_t, 16>{trace_bytes}};
    assert(trace_id == trace_id_copy);

    std::printf("opentelemetry trace_id test OK: trace=%.32s span=%.16s\n",
                trace_hex, span_hex);
    return 0;
}
