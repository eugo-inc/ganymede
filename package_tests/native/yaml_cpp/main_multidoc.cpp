/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yaml_cpp: multidoc; calls YAML::LoadAll().
 */

#include <yaml-cpp/yaml.h>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

int main() {
    const std::string stream =
        "---\n"
        "kind: first\n"
        "value: 1\n"
        "---\n"
        "kind: second\n"
        "value: 2\n"
        "---\n"
        "kind: third\n"
        "value: 3\n";

    std::vector<YAML::Node> docs = YAML::LoadAll(stream);
    assert(docs.size() == 3);
    assert(docs[0]["kind"].as<std::string>() == "first");
    assert(docs[0]["value"].as<int>() == 1);
    assert(docs[1]["kind"].as<std::string>() == "second");
    assert(docs[2]["value"].as<int>() == 3);

    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap << YAML::Key << "n" << YAML::Value << 10 << YAML::EndMap;
    out << YAML::EndDoc;
    out << YAML::BeginDoc;
    out << YAML::BeginMap << YAML::Key << "n" << YAML::Value << 20 << YAML::EndMap;
    out << YAML::EndDoc;
    assert(out.good());

    std::vector<YAML::Node> round_trip = YAML::LoadAll(out.c_str());
    assert(round_trip.size() == 2);
    assert(round_trip[0]["n"].as<int>() == 10);
    assert(round_trip[1]["n"].as<int>() == 20);

    /* Stream-overload of LoadAll — exercise the std::istream entry point. */
    std::istringstream iss(stream);
    std::vector<YAML::Node> from_stream = YAML::LoadAll(iss);
    assert(from_stream.size() == 3);
    assert(from_stream[2]["kind"].as<std::string>() == "third");

    std::printf("yaml_cpp multi-doc test OK: parsed %zu docs, round-tripped %zu\n",
                docs.size(), round_trip.size());
    return 0;
}
