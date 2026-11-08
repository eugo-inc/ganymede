/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for json_cpp: numeric; calls Json::writeString().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json/json.h>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>

int main(void) {
    /* 64-bit signed/unsigned construction + readback. */
    const int64_t big_signed = static_cast<int64_t>(9000000000LL);   /* > INT32_MAX */
    const uint64_t big_unsigned = static_cast<uint64_t>(18000000000ULL);

    Json::Value v_s(static_cast<Json::Int64>(big_signed));
    Json::Value v_u(static_cast<Json::UInt64>(big_unsigned));
    assert(v_s.isInt64());
    assert(v_u.isUInt64());
    assert(v_s.asInt64() == big_signed);
    assert(v_u.asUInt64() == big_unsigned);

    /* Negative 64-bit value should NOT report isUInt64() / isUInt(). */
    Json::Value v_neg(static_cast<Json::Int64>(-5000000000LL));
    assert(v_neg.isInt64());
    assert(!v_neg.isUInt64());
    assert(v_neg.asInt64() == -5000000000LL);

    const double pi_like = 3.141592653589793;
    Json::Value v_d(pi_like);
    assert(v_d.isDouble());
    assert(v_d.asDouble() == pi_like);

    Json::Value root(Json::objectValue);
    root["signed"] = v_s;
    root["unsigned"] = v_u;
    root["neg"] = v_neg;
    root["pi"] = v_d;

    Json::StreamWriterBuilder wb;
    wb["indentation"] = "";
    wb["precision"] = 17;
    wb["precisionType"] = "significant";
    const std::string out = Json::writeString(wb, root);
    assert(!out.empty());

    Json::CharReaderBuilder rb;
    std::unique_ptr<Json::CharReader> rdr(rb.newCharReader());
    Json::Value parsed;
    std::string errs;
    bool ok = rdr->parse(out.data(), out.data() + out.size(), &parsed, &errs);
    assert(ok);
    assert(errs.empty());

    assert(parsed["signed"].asInt64() == big_signed);
    assert(parsed["unsigned"].asUInt64() == big_unsigned);
    assert(parsed["neg"].asInt64() == -5000000000LL);

    assert(parsed["pi"].asDouble() == pi_like);

    Json::Value seven(7);
    assert(seven.isInt());
    assert(seven.type() == Json::intValue);
    assert(seven.type() != Json::realValue);
    assert(seven.asDouble() == 7.0);

    /* asString conversion on a numeric is supported via asString() on numeric values:
     * verify the documented behavior — jsoncpp converts numerics to their text form. */
    Json::Value forty_two(42);
    const std::string s = forty_two.asString();
    assert(s == "42");

    return 0;
}
