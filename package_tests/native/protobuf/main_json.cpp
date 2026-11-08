/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for protobuf: json; calls google::protobuf::util::MessageToJsonString(), google::protobuf::util::JsonStringToMessage(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/util/json_util.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    google::protobuf::FileDescriptorProto fdp;
    fdp.set_name("demo_json_probe.proto");
    fdp.set_package("demo.json");
    fdp.set_syntax("proto3");

    google::protobuf::DescriptorProto *msg = fdp.add_message_type();
    msg->set_name("JsonProbe");

    /* MessageToJsonString returns absl::Status — .ok() is the contract. */
    std::string json;
    auto status = google::protobuf::util::MessageToJsonString(fdp, &json);
    assert(status.ok());
    assert(!json.empty());
    assert(json.find("demo_json_probe.proto") != std::string::npos);
    assert(json.find("JsonProbe") != std::string::npos);

    google::protobuf::FileDescriptorProto parsed;
    status = google::protobuf::util::JsonStringToMessage(json, &parsed);
    assert(status.ok());

    assert(parsed.name() == "demo_json_probe.proto");
    assert(parsed.package() == "demo.json");
    assert(parsed.message_type_size() == 1);
    assert(parsed.message_type(0).name() == "JsonProbe");

    std::printf("protobuf json_util test OK: json_bytes=%zu\n", json.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
