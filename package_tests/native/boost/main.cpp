/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for boost: core API; calls fs::temp_directory_path(), fs::unique_path(), fs::create_directory(), fs::exists().
 */

#include <boost/version.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <cstdio>

int main() {
    assert(BOOST_VERSION >= 107000);

    /* libboost_system: default error_code is the "no error" state. */
    boost::system::error_code ec;
    assert(!ec);
    assert(ec.value() == 0);

    namespace fs = boost::filesystem;
    fs::path tmp = fs::temp_directory_path() / fs::unique_path("demo-boost-%%%%-%%%%");
    assert(fs::create_directory(tmp));
    assert(fs::exists(tmp));
    assert(fs::is_directory(tmp));
    assert(fs::remove(tmp));
    assert(!fs::exists(tmp));

    std::printf("boost test OK: %s\n", BOOST_LIB_VERSION);
    return 0;
}
