/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nlohmann_json: pointer; calls json::array(), json::diff(), json::object().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nlohmann/json.hpp>
#include <cassert>
#include <string>

using nlohmann::json;

int main() {
    json doc = {
        {"users", {
            {{"name", "alice"}, {"age", 30}},
            {{"name", "bob"},   {"age", 25}},
        }},
        {"meta", {{"version", 1}}},
    };

    /* RFC 6901 read: index into nested array element. */
    json::json_pointer p_bob_name("/users/1/name");
    assert(doc[p_bob_name].get<std::string>() == "bob");

    /* RFC 6901 write: in-place mutation through a pointer. */
    json::json_pointer p_meta_ver("/meta/version");
    doc[p_meta_ver] = 2;
    assert(doc["meta"]["version"].get<int>() == 2);

    const json patch = json::array({
        {{"op", "replace"}, {"path", "/users/0/age"}, {"value", 31}},
        {{"op", "add"},     {"path", "/users/0/email"}, {"value", "alice@example.org"}},
        {{"op", "remove"},  {"path", "/users/1/age"}},
        {{"op", "add"},     {"path", "/meta/tag"}, {"value", "hpc"}},
    });

    const json patched = doc.patch(patch);
    assert(patched["users"][0]["age"].get<int>() == 31);
    assert(patched["users"][0]["email"].get<std::string>() == "alice@example.org");
    assert(!patched["users"][1].contains("age"));
    assert(patched["meta"]["tag"].get<std::string>() == "hpc");

    const json diff = json::diff(doc, patched);
    assert(diff.is_array());
    assert(!diff.empty());
    const json reapplied = doc.patch(diff);
    assert(reapplied == patched);

    /* Pointer escape: tokens "/" and "~" must be encoded as ~1 and ~0. */
    json escaped = json::object();
    escaped["a/b"] = 1;
    escaped["c~d"] = 2;
    json::json_pointer p_slash("/a~1b");
    json::json_pointer p_tilde("/c~0d");
    assert(escaped[p_slash].get<int>() == 1);
    assert(escaped[p_tilde].get<int>() == 2);

    return 0;
}
