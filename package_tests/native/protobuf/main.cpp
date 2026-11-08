/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for protobuf: core API; calls google::protobuf::DescriptorPool::generated_pool(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
#ifndef GOOGLE_PROTOBUF_VERSION
#error "google/protobuf/stubs/common.h did not define GOOGLE_PROTOBUF_VERSION — broken install."
#endif

    /* The generated descriptor pool must contain descriptor.proto itself —
     * libprotobuf statically registers it during library init. If absl / utf8_range
     * linkage is broken, this returns nullptr (or the process aborts on init). */
    const google::protobuf::DescriptorPool *pool =
        google::protobuf::DescriptorPool::generated_pool();
    assert(pool != nullptr);

    const google::protobuf::FileDescriptor *file =
        pool->FindFileByName("google/protobuf/descriptor.proto");
    assert(file != nullptr);
    assert(file->message_type_count() > 0);

    /* FileDescriptorProto itself is defined in descriptor.proto — locate it by name. */
    const google::protobuf::Descriptor *fd_proto =
        pool->FindMessageTypeByName("google.protobuf.FileDescriptorProto");
    assert(fd_proto != nullptr);
    assert(std::string(fd_proto->name()) == "FileDescriptorProto");

    std::printf("protobuf test OK: version=%d\n", GOOGLE_PROTOBUF_VERSION);

    /* Cleanly shut down protobuf to avoid leak reports. */
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
