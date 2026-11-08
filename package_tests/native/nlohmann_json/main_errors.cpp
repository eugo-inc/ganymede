/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nlohmann_json: errors; calls json::parse(), json::array(), json::object().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nlohmann/json.hpp>
#include <cassert>
#include <string>

using nlohmann::json;

int main() {
    /* --- 1. Exception hierarchy --- */

    /* parse_error: ill-formed input. */
    bool parse_threw = false;
    try {
        (void) json::parse("{not valid json");
    } catch (const json::parse_error& e) {
        parse_threw = true;
        assert(e.id == 101 || e.id == 102 || e.id == 103);  /* parse_error.{101..103} */
    }
    assert(parse_threw);

    /* type_error: wrong-type get<T>. */
    bool type_threw = false;
    json scalar = 42;
    try {
        (void) scalar.get<std::string>();
    } catch (const json::type_error& e) {
        type_threw = true;
        assert(e.id >= 300);  /* type_error ids are 300-series. */
    }
    assert(type_threw);

    /* out_of_range: array index past end via at(). */
    bool oor_threw = false;
    json arr = json::array({10, 20, 30});
    try {
        (void) arr.at(99);
    } catch (const json::out_of_range& e) {
        oor_threw = true;
        assert(e.id >= 400);  /* out_of_range ids are 400-series. */
    }
    assert(oor_threw);

    /* --- 2. Type-introspection --- */
    json mixed = {
        {"obj_field", json::object()},
        {"arr_field", json::array()},
        {"str_field", "x"},
        {"int_field", 5},
        {"float_field", 1.5},
        {"bool_field", false},
        {"null_field", nullptr},
    };
    assert(mixed.is_object());
    assert(mixed["obj_field"].is_object());
    assert(mixed["arr_field"].is_array());
    assert(mixed["str_field"].is_string());
    assert(mixed["int_field"].is_number_integer());
    assert(mixed["float_field"].is_number_float());
    assert(mixed["bool_field"].is_boolean());
    assert(mixed["null_field"].is_null());

    /* type_name() returns the human-readable type label. */
    assert(std::string(mixed["str_field"].type_name()) == "string");
    assert(std::string(mixed["null_field"].type_name()) == "null");

    /* --- 3. Object iteration --- */
    json kv = {{"a", 1}, {"b", 2}, {"c", 3}};
    int sum = 0;
    int count = 0;
    for (auto it = kv.begin(); it != kv.end(); ++it) {
        sum += it.value().get<int>();
        const std::string k = it.key();
        assert(k == "a" || k == "b" || k == "c");
        ++count;
    }
    assert(count == 3);
    assert(sum == 6);

    /* items() range-for: alternative iteration entry-point. */
    int sum_items = 0;
    for (const auto& item : kv.items()) {
        sum_items += item.value().get<int>();
    }
    assert(sum_items == 6);

    return 0;
}
