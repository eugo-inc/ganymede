/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for protobuf: differencer; calls google::protobuf::util::MessageDifferencer::Equals(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/util/message_differencer.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    google::protobuf::FileDescriptorProto a;
    a.set_name("demo_diff_probe.proto");
    a.set_package("demo.diff");
    a.set_syntax("proto3");
    google::protobuf::DescriptorProto *am = a.add_message_type();
    am->set_name("DiffProbe");

    google::protobuf::FileDescriptorProto b;
    b.set_name("demo_diff_probe.proto");
    b.set_package("demo.diff");
    b.set_syntax("proto3");
    google::protobuf::DescriptorProto *bm = b.add_message_type();
    bm->set_name("DiffProbe");

    /* Equal messages: Equals must return true. */
    assert(google::protobuf::util::MessageDifferencer::Equals(a, b));

    /* Mutate b — Equals must now return false. */
    b.mutable_message_type(0)->set_name("DifferentName");
    assert(!google::protobuf::util::MessageDifferencer::Equals(a, b));

    /* ReportDifferencesToString captures the diff as a human-readable string. */
    std::string diff_report;
    {
        google::protobuf::util::MessageDifferencer differencer;
        differencer.ReportDifferencesToString(&diff_report);
        bool same = differencer.Compare(a, b);
        assert(!same);
    }
    assert(!diff_report.empty());
    /* The report should mention the differing field — either old or new value. */
    assert(diff_report.find("DiffProbe") != std::string::npos ||
           diff_report.find("DifferentName") != std::string::npos);

    /* Restore b to match a; Equals returns true again — proves the differencer
     * isn't latched into a permanent "different" state. */
    b.mutable_message_type(0)->set_name("DiffProbe");
    assert(google::protobuf::util::MessageDifferencer::Equals(a, b));

    std::printf("protobuf differencer test OK: report_bytes=%zu\n", diff_report.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
