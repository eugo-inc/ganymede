/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidjson: sax; compiles against <rapidjson/reader.h>, <rapidjson/stringbuffer.h>.
 */

#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct CountingHandler {
    int start_object = 0;
    int end_object = 0;
    int start_array = 0;
    int end_array = 0;
    int keys = 0;
    int strings = 0;
    int ints = 0;
    int bools = 0;
    std::vector<std::string> key_names;
    std::vector<int> int_values;

    bool Null() { return true; }
    bool Bool(bool) {
        ++bools;
        return true;
    }
    bool Int(int v) {
        ++ints;
        int_values.push_back(v);
        return true;
    }
    bool Uint(unsigned v) {
        ++ints;
        int_values.push_back(static_cast<int>(v));
        return true;
    }
    bool Int64(int64_t) { return true; }
    bool Uint64(uint64_t) { return true; }
    bool Double(double) { return true; }
    bool RawNumber(const char*, rapidjson::SizeType, bool) { return true; }
    bool String(const char*, rapidjson::SizeType, bool) {
        ++strings;
        return true;
    }
    bool StartObject() {
        ++start_object;
        return true;
    }
    bool Key(const char* str, rapidjson::SizeType length, bool) {
        ++keys;
        key_names.emplace_back(str, length);
        return true;
    }
    bool EndObject(rapidjson::SizeType) {
        ++end_object;
        return true;
    }
    bool StartArray() {
        ++start_array;
        return true;
    }
    bool EndArray(rapidjson::SizeType) {
        ++end_array;
        return true;
    }
};

int main() {
    const char json[] = "{\"a\":\"x\",\"b\":[1,2,3],\"c\":true}";
    rapidjson::StringStream ss(json);
    rapidjson::Reader reader;
    CountingHandler h;

    const bool ok = reader.Parse(ss, h);
    assert(ok);
    assert(!reader.HasParseError());

    /* Balanced object/array brackets. */
    assert(h.start_object == 1);
    assert(h.end_object == 1);
    assert(h.start_array == 1);
    assert(h.end_array == 1);

    /* 3 top-level keys, 1 string value, 3 ints, 1 bool. */
    assert(h.keys == 3);
    assert(h.strings == 1);
    assert(h.ints == 3);
    assert(h.bools == 1);

    /* Key names appear in document order. */
    assert(h.key_names.size() == 3);
    assert(h.key_names[0] == "a");
    assert(h.key_names[1] == "b");
    assert(h.key_names[2] == "c");

    /* Array values appear in document order. */
    assert(h.int_values.size() == 3);
    assert(h.int_values[0] == 1);
    assert(h.int_values[1] == 2);
    assert(h.int_values[2] == 3);

    std::printf("rapidjson SAX test OK: %d keys, %d ints\n", h.keys, h.ints);
    return 0;
}
