/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for protobuf: serialize; calls google::protobuf::DescriptorPool::generated_pool(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    const google::protobuf::DescriptorPool *pool =
        google::protobuf::DescriptorPool::generated_pool();
    assert(pool != nullptr);

    const google::protobuf::Descriptor *desc =
        pool->FindMessageTypeByName("google.protobuf.FileDescriptorProto");
    assert(desc != nullptr);

    google::protobuf::DynamicMessageFactory factory(pool);
    const google::protobuf::Message *prototype = factory.GetPrototype(desc);
    assert(prototype != nullptr);

    std::unique_ptr<google::protobuf::Message> msg(prototype->New());
    assert(msg != nullptr);

    /* Set the singular `name` field via reflection. */
    const google::protobuf::FieldDescriptor *name_field =
        desc->FindFieldByName("name");
    assert(name_field != nullptr);
    assert(name_field->type() == google::protobuf::FieldDescriptor::TYPE_STRING);

    const google::protobuf::Reflection *refl = msg->GetReflection();
    assert(refl != nullptr);
    refl->SetString(msg.get(), name_field, "demo_probe.proto");

    /* Serialize to wire format. */
    std::string wire;
    bool ok = msg->SerializeToString(&wire);
    assert(ok);
    assert(!wire.empty());

    std::unique_ptr<google::protobuf::Message> parsed(prototype->New());
    ok = parsed->ParseFromString(wire);
    assert(ok);

    const std::string &parsed_name = parsed->GetReflection()->GetString(*parsed, name_field);
    assert(parsed_name == "demo_probe.proto");

    std::printf("protobuf serialize test OK: wire_bytes=%zu\n", wire.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
