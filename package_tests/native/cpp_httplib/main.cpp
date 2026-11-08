/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cpp_httplib: core API; calls httplib::status_message().
 */

#include <httplib.h>
#include <cassert>
#include <cstring>
#include <cstdio>

int main(void) {
    const char *version = CPPHTTPLIB_VERSION;
    assert(version != nullptr);
    assert(std::strlen(version) > 0);

    /* status_message() returns the canonical reason phrase. */
    const char *ok = httplib::status_message(200);
    assert(ok != nullptr);
    assert(std::strcmp(ok, "OK") == 0);

    const char *not_found = httplib::status_message(404);
    assert(not_found != nullptr);
    assert(std::strcmp(not_found, "Not Found") == 0);

    std::printf("cpp_httplib test OK: %s\n", version);
    return 0;
}
