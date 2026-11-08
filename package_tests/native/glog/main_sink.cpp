/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for glog: sink; calls google::InitGoogleLogging(), google::AddLogSink(), google::RemoveLogSink() with a custom google::LogSink.
 */

#include <glog/logging.h>
#include <cassert>
#include <cstdio>

class CountingSink : public google::LogSink {
public:
    CountingSink() : count_(0) {}

    void send(google::LogSeverity severity,
              const char * /*full_filename*/,
              const char * /*base_filename*/,
              int /*line*/,
              const google::LogMessageTime & /*logmsgtime*/,
              const char * /*message*/,
              size_t /*message_len*/) override {
        ++count_;
        last_severity_ = severity;
    }

    int count() const { return count_; }
    google::LogSeverity last_severity() const { return last_severity_; }

private:
    int count_;
    google::LogSeverity last_severity_ = google::GLOG_INFO;
};

int main(int argc, char *argv[]) {
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = google::GLOG_INFO;
    google::InitGoogleLogging(argv[0]);

    CountingSink sink;
    assert(sink.count() == 0);

    google::AddLogSink(&sink);

    LOG(INFO) << "demo-glog sink probe: INFO #1";
    LOG(WARNING) << "demo-glog sink probe: WARNING #2";
    LOG(ERROR) << "demo-glog sink probe: ERROR #3";

    assert(sink.count() == 3);
    assert(sink.last_severity() == google::GLOG_ERROR);

    google::RemoveLogSink(&sink);

    /* After removal, the sink must stop receiving messages. */
    LOG(WARNING) << "demo-glog sink probe: post-removal (should NOT reach sink)";
    assert(sink.count() == 3);

    google::ShutdownGoogleLogging();
    std::printf("glog LogSink test OK\n");
    return 0;
}
