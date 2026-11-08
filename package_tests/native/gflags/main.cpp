/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gflags: core API; calls gflags::SetUsageMessage(), gflags::ParseCommandLineFlags(), gflags::ShutDownCommandLineFlags().
 */

#include <gflags/gflags.h>
#include <cassert>
#include <iostream>

int main(int argc, char **argv) {
    gflags::SetUsageMessage("demo gflags smoke");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    gflags::ShutDownCommandLineFlags();
    std::cout << "gflags test OK" << std::endl;
    return 0;
}
