/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidjson: parse errors; calls rapidjson::GetParseError_En().
 */

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    /* 1. Trailing-comma error — invalid JSON. */
    {
        rapidjson::Document d;
        d.Parse("{\"a\":1,}");
        assert(d.HasParseError());

        const rapidjson::ParseErrorCode code = d.GetParseError();
        assert(code != rapidjson::kParseErrorNone);

        /* The English translation must be non-null and non-empty. */
        const char* msg = rapidjson::GetParseError_En(code);
        assert(msg != nullptr);
        assert(std::strlen(msg) > 0);

        /* Error offset is positive (parser stalled past the start). */
        const size_t offset = d.GetErrorOffset();
        assert(offset > 0);
    }

    /* 2. Unterminated string — distinct error code from trailing-comma. */
    {
        rapidjson::Document d1;
        d1.Parse("{\"a\":1,}");
        const rapidjson::ParseErrorCode code_trailing = d1.GetParseError();

        rapidjson::Document d2;
        d2.Parse("{\"a\":\"unterminated");
        assert(d2.HasParseError());
        const rapidjson::ParseErrorCode code_unterm = d2.GetParseError();

        /* Two structurally distinct errors should surface distinct codes. */
        assert(code_trailing != code_unterm);
    }

    /* 3. Empty input — parser flags this. */
    {
        rapidjson::Document d;
        d.Parse("");
        assert(d.HasParseError());
        assert(d.GetParseError() == rapidjson::kParseErrorDocumentEmpty);
    }

    /* 4. ParseInsitu: in-place parse of a writable buffer. */
    {
        char json[] = "{\"k\":\"value\",\"n\":17}";
        rapidjson::Document d;
        d.ParseInsitu(json);
        assert(!d.HasParseError());
        assert(d.IsObject());
        assert(d.HasMember("k"));
        assert(d["k"].IsString());
        assert(std::strcmp(d["k"].GetString(), "value") == 0);
        assert(d.HasMember("n"));
        assert(d["n"].GetInt() == 17);
    }

    /* 5. After a successful parse, GetParseError must be kParseErrorNone. */
    {
        rapidjson::Document d;
        d.Parse("{}");
        assert(!d.HasParseError());
        assert(d.GetParseError() == rapidjson::kParseErrorNone);
    }

    std::printf("rapidjson parse-errors test OK\n");
    return 0;
}
