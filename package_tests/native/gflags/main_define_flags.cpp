/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for gflags: define flags; calls DEFINE_int32(), DEFINE_bool(), DEFINE_string(), DEFINE_double().
 */

#include <gflags/gflags.h>
#include <cassert>
#include <cstring>
#include <string>

DEFINE_int32(demo_int, 7, "an int flag");
DEFINE_bool(demo_bool, false, "a bool flag");
DEFINE_string(demo_str, "default", "a string flag");
DEFINE_double(demo_dbl, 1.5, "a double flag");

int main(int argc, char **argv) {
    // Defaults must be visible BEFORE parsing.
    assert(FLAGS_demo_int == 7);
    assert(FLAGS_demo_bool == false);
    assert(FLAGS_demo_str == "default");
    assert(FLAGS_demo_dbl == 1.5);

    const char *fake[] = {
        "prog",
        "--demo_int=42",
        "--demo_bool=true",
        "--demo_str=hello",
        "--demo_dbl=2.75",
        nullptr,
    };
    int fake_argc = 5;
    char **fake_argv = const_cast<char **>(fake);
    gflags::ParseCommandLineFlags(&fake_argc, &fake_argv, false);

    // Post-parse values must reflect argv overrides.
    assert(FLAGS_demo_int == 42);
    assert(FLAGS_demo_bool == true);
    assert(FLAGS_demo_str == "hello");
    assert(FLAGS_demo_dbl == 2.75);

    gflags::ShutDownCommandLineFlags();
    (void)argc;
    (void)argv;
    return 0;
}
