/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidjson: core API; compiles against <rapidjson/document.h>, <rapidjson/rapidjson.h>.
 */

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    /* Header sanity: rapidjson exposes a major-version macro since the 1.x line. */
    assert(RAPIDJSON_MAJOR_VERSION >= 1);

    const char json[] = "{\"name\":\"demo\",\"count\":42,\"ok\":true}";
    rapidjson::Document d;
    d.Parse(json);
    assert(!d.HasParseError());
    assert(d.IsObject());

    assert(d.HasMember("name") && d["name"].IsString());
    assert(std::strcmp(d["name"].GetString(), "demo") == 0);

    assert(d.HasMember("count") && d["count"].IsInt());
    assert(d["count"].GetInt() == 42);

    assert(d.HasMember("ok") && d["ok"].IsBool());
    assert(d["ok"].GetBool() == true);

    std::printf("rapidjson test OK: %d.%d.%d\n",
                RAPIDJSON_MAJOR_VERSION, RAPIDJSON_MINOR_VERSION, RAPIDJSON_PATCH_VERSION);
    return 0;
}
