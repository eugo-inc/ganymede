/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for json_cpp: core API; calls Json::writeString().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json/json.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

int main(void) {
    const std::string input = R"({"name":"demo","count":42,"flag":true})";

    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    Json::Value root;
    std::string errs;
    bool ok = reader->parse(input.data(), input.data() + input.size(), &root, &errs);
    assert(ok);
    assert(errs.empty());

    assert(root["name"].asString() == "demo");
    assert(root["count"].asInt() == 42);
    assert(root["flag"].asBool() == true);

    Json::StreamWriterBuilder writer_builder;
    writer_builder["indentation"] = "";
    const std::string serialized = Json::writeString(writer_builder, root);
    assert(!serialized.empty());

    Json::Value reparsed;
    ok = reader->parse(serialized.data(), serialized.data() + serialized.size(), &reparsed, &errs);
    assert(ok);
    assert(reparsed == root);

    std::cout << "json_cpp test OK: " << JSONCPP_VERSION_STRING << std::endl;
    return 0;
}
