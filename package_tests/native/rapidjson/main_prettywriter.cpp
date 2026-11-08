/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rapidjson: prettywriter; compiles against <rapidjson/prettywriter.h>, <rapidjson/stringbuffer.h>.
 */

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    /* Default indent: 4 spaces. */
    {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);

        w.StartObject();
        w.Key("a");
        w.Int(1);
        w.Key("b");
        w.StartArray();
        w.Int(2);
        w.Int(3);
        w.EndArray();
        w.EndObject();

        const char* out = sb.GetString();
        assert(out != nullptr);

        /* Pretty output MUST contain newlines (compact Writer would not). */
        assert(std::strchr(out, '\n') != nullptr);

        /* Default indent is 4 spaces — "    \"a\":" should appear. */
        assert(std::strstr(out, "    \"a\": 1") != nullptr);
        assert(std::strstr(out, "    \"b\": [") != nullptr);

        /* Nested array entries get 8 spaces (2 levels * 4). */
        assert(std::strstr(out, "        2,") != nullptr);
        assert(std::strstr(out, "        3") != nullptr);
    }

    /* Custom indent: 2 spaces. SetIndent returns the writer by ref. */
    {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
        w.SetIndent(' ', 2);

        w.StartObject();
        w.Key("x");
        w.Int(7);
        w.EndObject();

        const char* out = sb.GetString();
        assert(out != nullptr);

        /* 2-space indent: "  \"x\":" appears, but NOT "    \"x\":" (4-space). */
        assert(std::strstr(out, "  \"x\": 7") != nullptr);
        assert(std::strstr(out, "    \"x\"") == nullptr);
    }

    /* Custom indent: tab character. */
    {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
        w.SetIndent('\t', 1);

        w.StartObject();
        w.Key("k");
        w.String("v");
        w.EndObject();

        const char* out = sb.GetString();
        assert(out != nullptr);
        /* Tab MUST appear in the output. */
        assert(std::strchr(out, '\t') != nullptr);
    }

    std::printf("rapidjson prettywriter test OK\n");
    return 0;
}
