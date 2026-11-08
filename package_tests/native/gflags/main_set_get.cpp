/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gflags: set get; calls DEFINE_int32(), DEFINE_string(), gflags::GetCommandLineOption(), gflags::SetCommandLineOption().
 */

#include <gflags/gflags.h>
#include <cassert>
#include <string>

DEFINE_int32(demo_sg_count, 10, "count for set/get aspect");
DEFINE_string(demo_sg_name, "alpha", "name for set/get aspect");

int main(int /*argc*/, char ** /*argv*/) {
    // GetCommandLineOption returns "true" on success and writes into out.
    std::string out;
    bool ok = gflags::GetCommandLineOption("demo_sg_count", &out);
    assert(ok);
    assert(out == "10");

    // SetCommandLineOption returns a non-empty string on success
    // (the human-readable confirmation line); empty on failure.
    std::string ret = gflags::SetCommandLineOption("demo_sg_count", "99");
    assert(!ret.empty());
    ret = gflags::SetCommandLineOption("demo_sg_name", "omega");
    assert(!ret.empty());

    ok = gflags::GetCommandLineOption("demo_sg_count", &out);
    assert(ok);
    assert(out == "99");
    ok = gflags::GetCommandLineOption("demo_sg_name", &out);
    assert(ok);
    assert(out == "omega");

    // FLAGS_ accessor must reflect the programmatic change.
    assert(FLAGS_demo_sg_count == 99);
    assert(FLAGS_demo_sg_name == "omega");

    // CommandLineFlagInfo introspection.
    gflags::CommandLineFlagInfo info;
    bool found = gflags::GetCommandLineFlagInfo("demo_sg_count", &info);
    assert(found);
    assert(info.name == "demo_sg_count");
    assert(info.type == "int32");
    assert(info.default_value == "10");
    assert(info.current_value == "99");
    assert(info.is_default == false);  // mutated above

    // Nonexistent flag must return false.
    ok = gflags::GetCommandLineOption("definitely_not_a_flag", &out);
    assert(!ok);

    return 0;
}
