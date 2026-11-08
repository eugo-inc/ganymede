/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for flatbuffers: parser; compiles against <flatbuffers/idl.h>.
 */

#include <flatbuffers/idl.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* A minimal but valid FlatBuffers schema describing a Monster table. */
    const std::string schema_ok =
        "table Monster {\n"
        "  hp: int = 100;\n"
        "  name: string;\n"
        "}\n"
        "root_type Monster;\n";

    /* A deliberately broken schema — missing closing brace + semicolons. */
    const std::string schema_bad =
        "table Monster {\n"
        "  hp int = 100\n"
        "  name string\n";

    /* Well-formed JSON matching the schema. */
    const std::string json_ok =
        "{ \"hp\": 42, \"name\": \"demo-parser-probe\" }";

    /* Parser must accept the valid schema. */
    {
        flatbuffers::Parser parser;
        bool ok = parser.Parse(schema_ok.c_str());
        assert(ok && "Parser rejected a well-formed schema");
    }

    /* Parser must reject the broken schema. */
    {
        flatbuffers::Parser parser;
        bool ok = parser.Parse(schema_bad.c_str());
        assert(!ok && "Parser accepted a broken schema");
    }

    /* Parser must parse the schema then accept the JSON instance, producing
     * a non-empty serialized buffer. */
    {
        flatbuffers::Parser parser;
        bool schema_ok_2 = parser.Parse(schema_ok.c_str());
        assert(schema_ok_2);
        bool json_ok_parsed = parser.Parse(json_ok.c_str());
        assert(json_ok_parsed && "Parser rejected a well-formed JSON instance");
        assert(parser.builder_.GetSize() > 0 && "Parser produced an empty buffer");
    }

    std::printf("flatbuffers parser test OK\n");
    return 0;
}
