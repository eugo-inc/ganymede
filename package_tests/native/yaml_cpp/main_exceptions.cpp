/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yaml_cpp: exceptions; calls YAML::Load().
 */

#include <yaml-cpp/yaml.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* Malformed YAML must throw a ParserException (or its Exception base). */
    bool parser_threw = false;
    try {
        /* Unbalanced flow-sequence — '[' with no closing ']'. */
        YAML::Node bad = YAML::Load("key: [1, 2, 3\nother: value");
        (void)bad;
    } catch (const YAML::ParserException &e) {
        parser_threw = true;
        assert(std::string(e.what()).length() > 0);
    } catch (const YAML::Exception &e) {
        /* Acceptable: ParserException derives from Exception. */
        parser_threw = true;
        assert(std::string(e.what()).length() > 0);
    }
    assert(parser_threw);

    /* Type-mismatched accessor must throw BadConversion (or Exception base). */
    YAML::Node doc = YAML::Load("name: demo\n");
    bool conv_threw = false;
    try {
        int bogus = doc["name"].as<int>();
        (void)bogus;
    } catch (const YAML::BadConversion &e) {
        conv_threw = true;
        assert(std::string(e.what()).length() > 0);
    } catch (const YAML::Exception &e) {
        conv_threw = true;
        assert(std::string(e.what()).length() > 0);
    }
    assert(conv_threw);

    std::printf("yaml_cpp exceptions test OK: both error paths threw as expected\n");
    return 0;
}
