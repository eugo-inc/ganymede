/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidjson: writer; compiles against <rapidjson/stringbuffer.h>, <rapidjson/writer.h>.
 */

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);

    w.StartObject();
    w.Key("name");
    w.String("demo");
    w.Key("count");
    w.Int(42);
    w.Key("ok");
    w.Bool(true);
    w.Key("ratio");
    w.Double(0.5);
    w.Key("nums");
    w.StartArray();
    w.Int(1);
    w.Int(2);
    w.Int(3);
    w.EndArray();
    w.EndObject();

    const char* out = sb.GetString();
    assert(out != nullptr);

    const char expected[] =
        "{\"name\":\"demo\",\"count\":42,\"ok\":true,\"ratio\":0.5,\"nums\":[1,2,3]}";
    assert(std::strcmp(out, expected) == 0);
    assert(sb.GetSize() == std::strlen(expected));

    std::printf("rapidjson writer test OK: %zu bytes\n", sb.GetSize());
    return 0;
}
