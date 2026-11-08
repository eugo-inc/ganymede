/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidjson: pointer; calls rapidjson::Pointer().
 */

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    const char json[] =
        "{\"users\":[{\"name\":\"alice\",\"age\":30},{\"name\":\"bob\",\"age\":25}],"
        "\"meta\":{\"count\":2}}";
    rapidjson::Document d;
    d.Parse(json);
    assert(!d.HasParseError());

    /* Read with Pointer::Get — returns nullptr on miss, pointer to the Value on hit. */
    const rapidjson::Value* count = rapidjson::Pointer("/meta/count").Get(d);
    assert(count != nullptr);
    assert(count->IsInt());
    assert(count->GetInt() == 2);

    /* Array indexing via /N/. */
    const rapidjson::Value* alice_name = rapidjson::Pointer("/users/0/name").Get(d);
    assert(alice_name != nullptr);
    assert(alice_name->IsString());
    assert(std::strcmp(alice_name->GetString(), "alice") == 0);

    const rapidjson::Value* bob_age = rapidjson::Pointer("/users/1/age").Get(d);
    assert(bob_age != nullptr);
    assert(bob_age->IsInt());
    assert(bob_age->GetInt() == 25);

    /* Miss should yield nullptr (not throw). */
    const rapidjson::Value* missing = rapidjson::Pointer("/users/2/name").Get(d);
    assert(missing == nullptr);

    /* Mutation via Pointer::Set — auto-creates intermediate path components. */
    rapidjson::Pointer("/meta/note").Set(d, "added-by-demo");
    const rapidjson::Value* note = rapidjson::Pointer("/meta/note").Get(d);
    assert(note != nullptr);
    assert(note->IsString());
    assert(std::strcmp(note->GetString(), "added-by-demo") == 0);

    /* Deep auto-vivification. */
    rapidjson::Pointer("/extra/nested/key").Set(d, 99);
    const rapidjson::Value* deep = rapidjson::Pointer("/extra/nested/key").Get(d);
    assert(deep != nullptr);
    assert(deep->IsInt());
    assert(deep->GetInt() == 99);

    std::printf("rapidjson pointer test OK\n");
    return 0;
}
