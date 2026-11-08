/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for protobuf: textformat; calls google::protobuf::TextFormat::PrintToString(), google::protobuf::TextFormat::ParseFromString(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/text_format.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* Use the strongly-typed generated FileDescriptorProto class — its symbols
     * live in libprotobuf and exercise the linked-in generated descriptor pool. */
    google::protobuf::FileDescriptorProto fdp;
    fdp.set_name("demo_textformat_probe.proto");
    fdp.set_package("demo.test");
    fdp.set_syntax("proto3");

    google::protobuf::DescriptorProto *msg = fdp.add_message_type();
    msg->set_name("Probe");
    google::protobuf::FieldDescriptorProto *field = msg->add_field();
    field->set_name("payload");
    field->set_number(1);
    field->set_type(google::protobuf::FieldDescriptorProto::TYPE_STRING);
    field->set_label(google::protobuf::FieldDescriptorProto::LABEL_OPTIONAL);

    /* Print to text format. */
    std::string text;
    bool ok = google::protobuf::TextFormat::PrintToString(fdp, &text);
    assert(ok);
    assert(!text.empty());
    assert(text.find("demo_textformat_probe.proto") != std::string::npos);
    assert(text.find("Probe") != std::string::npos);
    assert(text.find("payload") != std::string::npos);

    google::protobuf::FileDescriptorProto parsed;
    ok = google::protobuf::TextFormat::ParseFromString(text, &parsed);
    assert(ok);

    assert(parsed.name() == "demo_textformat_probe.proto");
    assert(parsed.package() == "demo.test");
    assert(parsed.message_type_size() == 1);
    assert(parsed.message_type(0).name() == "Probe");
    assert(parsed.message_type(0).field_size() == 1);
    assert(parsed.message_type(0).field(0).name() == "payload");
    assert(parsed.message_type(0).field(0).number() == 1);

    std::printf("protobuf textformat test OK: text_bytes=%zu\n", text.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
