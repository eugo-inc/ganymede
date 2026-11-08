/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for json_cpp: errors; calls Json::CharReaderBuilder::strictMode().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json/json.h>
#include <cassert>
#include <memory>
#include <string>

int main(void) {
    Json::CharReaderBuilder rb;
    std::unique_ptr<Json::CharReader> rdr(rb.newCharReader());

    /* 1. Truncated object: must fail with a non-empty error message. */
    {
        const std::string bad = R"({"a": 1, "b": )";
        Json::Value root;
        std::string errs;
        bool ok = rdr->parse(bad.data(), bad.data() + bad.size(), &root, &errs);
        assert(!ok);
        assert(!errs.empty());
    }

    /* 2. Unterminated string: must fail with a non-empty error message. */
    {
        const std::string bad = R"({"a": "unterminated)";
        Json::Value root;
        std::string errs;
        bool ok = rdr->parse(bad.data(), bad.data() + bad.size(), &root, &errs);
        assert(!ok);
        assert(!errs.empty());
    }

    /* 3. Strict mode rejects comments, which the relaxed default builder
     *    accepts. Construct a strict reader. */
    {
        Json::CharReaderBuilder strict;
        Json::CharReaderBuilder::strictMode(&strict.settings_);
        std::unique_ptr<Json::CharReader> srdr(strict.newCharReader());
        const std::string with_comment = R"({"a": 1 /* trailing comment */ })";
        Json::Value root;
        std::string errs;
        bool ok = srdr->parse(with_comment.data(),
                              with_comment.data() + with_comment.size(),
                              &root, &errs);
        /* Strict mode forbids comments. */
        assert(!ok);
        assert(!errs.empty());
    }

    /* 4. Relaxed default ACCEPTS comments — confirm the opposite path. */
    {
        const std::string with_comment = R"({"a": 1 /* allowed by default */ })";
        Json::Value root;
        std::string errs;
        bool ok = rdr->parse(with_comment.data(),
                             with_comment.data() + with_comment.size(),
                             &root, &errs);
        assert(ok);
        assert(root["a"].asInt() == 1);
    }

    /* 5. Empty input is invalid JSON. */
    {
        const std::string empty;
        Json::Value root;
        std::string errs;
        bool ok = rdr->parse(empty.data(), empty.data() + empty.size(), &root, &errs);
        assert(!ok);
    }

    return 0;
}
