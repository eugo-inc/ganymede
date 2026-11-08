/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for boost: json; calls json::parse(), json::serialize().
 */

#include <boost/json.hpp>

#include <cassert>
#include <cstdio>
#include <string>

int main() {
    namespace json = boost::json;

    json::value v = json::parse(R"({"name":"demo","count":42,"ok":true,"ratio":1.5})");
    assert(v.is_object());

    const json::object& o = v.as_object();
    assert(o.at("name").as_string() == "demo");
    assert(o.at("count").as_int64() == 42);
    assert(o.at("ok").as_bool() == true);
    assert(o.at("ratio").as_double() > 1.49 && o.at("ratio").as_double() < 1.51);

    /* Negative: malformed JSON must throw, not silently succeed. */
    bool threw = false;
    try {
        (void)json::parse("{not json");
    } catch (const std::exception&) {
        threw = true;
    }
    assert(threw);

    const std::string s = json::serialize(v);
    json::value v2 = json::parse(s);
    assert(v2.as_object().at("count").as_int64() == 42);
    assert(v2.as_object().at("name").as_string() == "demo");

    std::printf("boost json test OK\n");
    return 0;
}
