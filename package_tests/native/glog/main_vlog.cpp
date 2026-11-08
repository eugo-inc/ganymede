/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for glog: vlog; calls google::InitGoogleLogging(), google::SetVLOGLevel(), google::ShutdownGoogleLogging().
 */

#include <glog/logging.h>
#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = google::GLOG_ERROR;
    google::InitGoogleLogging(argv[0]);

    /* By default, no verbose levels are on. */
    FLAGS_v = 0;
    assert(!VLOG_IS_ON(1));
    assert(!VLOG_IS_ON(2));

    /* Bump the global verbose level to 2. Both VLOG(1) and VLOG(2) must be on,
     * VLOG(3) must still be off. */
    FLAGS_v = 2;
    assert(VLOG_IS_ON(1));
    assert(VLOG_IS_ON(2));
    assert(!VLOG_IS_ON(3));

    VLOG(1) << "demo-glog VLOG(1) enabled at FLAGS_v=2";
    VLOG(2) << "demo-glog VLOG(2) enabled at FLAGS_v=2";
    VLOG(3) << "demo-glog VLOG(3) NOT enabled at FLAGS_v=2 (should be suppressed)";

    /* Per-module override: bump THIS source file ("main_vlog") to level 4.
     * SetVLOGLevel returns the previous level for the matched module (0 if none). */
    int previous = google::SetVLOGLevel("main_vlog", 4);
    (void)previous;
    assert(VLOG_IS_ON(3));
    assert(VLOG_IS_ON(4));
    assert(!VLOG_IS_ON(5));

    /* Restore. */
    FLAGS_v = 0;
    google::SetVLOGLevel("main_vlog", 0);

    google::ShutdownGoogleLogging();
    std::printf("glog VLOG test OK\n");
    return 0;
}
