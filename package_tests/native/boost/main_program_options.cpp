/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for boost: program options; calls po::value(), po::store(), po::parse_command_line(), po::notify().
 */

#include <boost/program_options.hpp>
#include <cassert>
#include <cstdio>
#include <string>

namespace po = boost::program_options;

int main() {
    po::options_description desc("test options");
    desc.add_options()
        ("verbose,v", "enable verbose output")
        ("name", po::value<std::string>()->default_value("demo"), "name string")
        ("count", po::value<int>(), "an integer count");

    /* Synthetic argv: --verbose --name=boost --count 7 */
    const char* argv[] = {"prog", "--verbose", "--name=boost", "--count", "7"};
    const int argc = 5;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
    po::notify(vm);

    assert(vm.count("verbose") == 1);
    assert(vm.count("name") == 1);
    assert(vm["name"].as<std::string>() == "boost");
    assert(vm.count("count") == 1);
    assert(vm["count"].as<int>() == 7);

    /* Default-value path: a second parse with neither --name nor --count. */
    const char* argv2[] = {"prog", "-v"};
    po::variables_map vm2;
    po::store(po::parse_command_line(2, const_cast<char**>(argv2), desc), vm2);
    po::notify(vm2);

    assert(vm2.count("verbose") == 1);
    assert(vm2["name"].as<std::string>() == "demo"); /* default kicked in */
    assert(vm2.count("count") == 0); /* no default → absent */

    std::printf("boost program_options test OK\n");
    return 0;
}
