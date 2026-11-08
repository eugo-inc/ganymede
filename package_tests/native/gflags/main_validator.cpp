/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for gflags: validator; calls DEFINE_int32(), gflags::RegisterFlagValidator(), gflags::SetCommandLineOption().
 */

#include <gflags/gflags.h>
#include <cassert>
#include <string>

DEFINE_int32(demo_port, 8080, "port flag with validator");

static bool ValidatePort(const char * /*flagname*/, int32_t value) {
    return value > 0 && value < 65536;
}

int main(int /*argc*/, char ** /*argv*/) {
    bool registered = gflags::RegisterFlagValidator(&FLAGS_demo_port, &ValidatePort);
    assert(registered);

    // In-range write — validator passes, SetCommandLineOption returns non-empty.
    std::string ret = gflags::SetCommandLineOption("demo_port", "9090");
    assert(!ret.empty());
    assert(FLAGS_demo_port == 9090);

    // Out-of-range write — validator rejects, returns empty string, value unchanged.
    ret = gflags::SetCommandLineOption("demo_port", "70000");
    assert(ret.empty());
    assert(FLAGS_demo_port == 9090);

    ret = gflags::SetCommandLineOption("demo_port", "-1");
    assert(ret.empty());
    assert(FLAGS_demo_port == 9090);

    // Edge-of-range still rejects per validator (value == 65536 is invalid).
    ret = gflags::SetCommandLineOption("demo_port", "65536");
    assert(ret.empty());
    assert(FLAGS_demo_port == 9090);

    // Re-registering the same validator is allowed; the result is not checked.
    bool replaced = gflags::RegisterFlagValidator(&FLAGS_demo_port, &ValidatePort);
    (void)replaced;

    return 0;
}
