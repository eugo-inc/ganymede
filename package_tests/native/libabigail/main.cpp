/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: core API; calls abigail::tools_utils::initialize(), abigail::tools_utils::get_library_version_string(), abigail::tools_utils::get_abixml_version_string().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-tools-utils.h>
#include <cassert>
#include <iostream>
#include <string>

int main(void) {
    abigail::tools_utils::initialize();

    const std::string version = abigail::tools_utils::get_library_version_string();
    assert(!version.empty());
    assert(version.find('.') != std::string::npos);

    const std::string abixml_version = abigail::tools_utils::get_abixml_version_string();
    assert(!abixml_version.empty());
    assert(abixml_version.find('.') != std::string::npos);

    std::cout << "libabigail test OK: " << version
              << " (abixml " << abixml_version << ")" << std::endl;
    return 0;
}
