/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for glog: core API; calls google::IsGoogleLoggingInitialized(), google::InitGoogleLogging(), google::ShutdownGoogleLogging().
 */

#include <glog/logging.h>
#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    /* Quiet glog: send to stderr only, suppress INFO from stderr so the test output stays clean. */
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = google::GLOG_WARNING;

    assert(!google::IsGoogleLoggingInitialized());
    google::InitGoogleLogging(argv[0]);
    assert(google::IsGoogleLoggingInitialized());

    /* Exercise the core LOG macro — links + dispatches through the sink machinery. */
    LOG(INFO) << "demo-glog-smoke INFO line (suppressed by minloglevel)";
    LOG(WARNING) << "demo-glog-smoke WARNING line";

    google::ShutdownGoogleLogging();
    assert(!google::IsGoogleLoggingInitialized());

    std::printf("glog test OK\n");
    return 0;
}
