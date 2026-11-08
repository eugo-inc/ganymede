/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rapidjson: schema; compiles against <rapidjson/document.h>, <rapidjson/schema.h>, <rapidjson/stringbuffer.h>.
 */

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    /* JSON Schema: object with required "name" (string) and "age" (integer >= 0). */
    const char schema_json[] =
        "{"
        "\"type\":\"object\","
        "\"properties\":{"
        "  \"name\":{\"type\":\"string\"},"
        "  \"age\":{\"type\":\"integer\",\"minimum\":0}"
        "},"
        "\"required\":[\"name\",\"age\"]"
        "}";

    rapidjson::Document schema_doc;
    schema_doc.Parse(schema_json);
    assert(!schema_doc.HasParseError());
    rapidjson::SchemaDocument schema(schema_doc);

    /* Happy path: valid document. */
    {
        rapidjson::Document d;
        d.Parse("{\"name\":\"demo\",\"age\":7}");
        assert(!d.HasParseError());

        rapidjson::SchemaValidator validator(schema);
        const bool ok = d.Accept(validator);
        assert(ok);
        assert(validator.IsValid());
    }

    /* Failure path 1: missing required field. */
    {
        rapidjson::Document d;
        d.Parse("{\"name\":\"demo\"}");
        assert(!d.HasParseError());

        rapidjson::SchemaValidator validator(schema);
        const bool ok = d.Accept(validator);
        assert(!ok);
        assert(!validator.IsValid());

        /* InvalidSchemaKeyword pinpoints WHICH constraint failed — "required" here. */
        const char* kw = validator.GetInvalidSchemaKeyword();
        assert(kw != nullptr);
        assert(std::strcmp(kw, "required") == 0);
    }

    /* Failure path 2: wrong type. */
    {
        rapidjson::Document d;
        d.Parse("{\"name\":\"demo\",\"age\":\"seven\"}");
        assert(!d.HasParseError());

        rapidjson::SchemaValidator validator(schema);
        const bool ok = d.Accept(validator);
        assert(!ok);
        assert(!validator.IsValid());

        const char* kw = validator.GetInvalidSchemaKeyword();
        assert(kw != nullptr);
        assert(std::strcmp(kw, "type") == 0);
    }

    /* Failure path 3: numeric constraint violation. */
    {
        rapidjson::Document d;
        d.Parse("{\"name\":\"demo\",\"age\":-1}");
        assert(!d.HasParseError());

        rapidjson::SchemaValidator validator(schema);
        const bool ok = d.Accept(validator);
        assert(!ok);

        const char* kw = validator.GetInvalidSchemaKeyword();
        assert(kw != nullptr);
        assert(std::strcmp(kw, "minimum") == 0);
    }

    std::printf("rapidjson schema test OK\n");
    return 0;
}
