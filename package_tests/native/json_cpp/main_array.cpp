/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for json_cpp: array; calls Json::Value().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json/json.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

int main(void) {
    Json::Value arr(Json::arrayValue);
    assert(arr.isArray());
    assert(arr.size() == 0);
    assert(arr.empty());

    arr.append(Json::Value(10));
    arr.append(Json::Value(20));
    arr.append(Json::Value(30));
    assert(arr.size() == 3);
    assert(!arr.empty());
    assert(arr[0].asInt() == 10);
    assert(arr[2].asInt() == 30);

    /* Mutate via index assignment. */
    arr[1] = Json::Value(99);
    assert(arr[1].asInt() == 99);

    /* Iterate via const_iterator and sum. */
    long sum = 0;
    for (Json::Value::const_iterator it = arr.begin(); it != arr.end(); ++it) {
        sum += it->asInt();
    }
    assert(sum == 10 + 99 + 30);

    Json::Value obj(Json::objectValue);
    obj["values"] = arr;
    obj["meta"] = Json::Value(Json::objectValue);
    obj["meta"]["owner"] = "demo";
    obj["meta"]["temp"] = "deleteme";

    assert(obj.isObject());
    assert(obj.isMember("values"));
    assert(obj.isMember("meta"));
    assert(obj["values"].isArray());
    assert(obj["values"].size() == 3);
    assert(obj["meta"]["owner"].asString() == "demo");

    /* getMemberNames + removeMember. */
    std::vector<std::string> names = obj["meta"].getMemberNames();
    assert(names.size() == 2);
    assert(std::find(names.begin(), names.end(), std::string("owner")) != names.end());
    assert(std::find(names.begin(), names.end(), std::string("temp")) != names.end());

    Json::Value removed;
    bool did_remove = obj["meta"].removeMember("temp", &removed);
    assert(did_remove);
    assert(removed.asString() == "deleteme");
    assert(!obj["meta"].isMember("temp"));
    assert(obj["meta"].getMemberNames().size() == 1);

    /* Parse an array literal back through the reader to confirm symmetric handling. */
    Json::CharReaderBuilder rb;
    Json::Value parsed;
    std::string errs;
    const std::string src = "[1,2,3,4,5]";
    std::unique_ptr<Json::CharReader> rdr(rb.newCharReader());
    bool ok = rdr->parse(src.data(), src.data() + src.size(), &parsed, &errs);
    assert(ok);
    assert(parsed.isArray());
    assert(parsed.size() == 5);
    assert(parsed[4].asInt() == 5);

    return 0;
}
