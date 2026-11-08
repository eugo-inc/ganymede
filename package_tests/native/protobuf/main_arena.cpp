/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for protobuf: arena; calls google::protobuf::Arena::Create(), google::protobuf::ShutdownProtobufLibrary().
 */

#include <google/protobuf/arena.h>
#include <google/protobuf/descriptor.pb.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    google::protobuf::Arena arena;

    /* Allocate a FileDescriptorProto entirely inside the arena.
     * Arena::Create<T>(Arena*) is the canonical arena allocation entry point
     * (the older CreateMessage<T> alias was removed in protobuf 7.x). */
    google::protobuf::FileDescriptorProto *fdp =
        google::protobuf::Arena::Create<google::protobuf::FileDescriptorProto>(&arena);
    assert(fdp != nullptr);

    /* Arena-allocated messages MUST report the owning arena via GetArena(). */
    assert(fdp->GetArena() == &arena);

    /* Submessages added via add_* are also arena-allocated on the same arena. */
    fdp->set_name("demo_arena_probe.proto");
    fdp->set_package("demo.arena");
    google::protobuf::DescriptorProto *msg = fdp->add_message_type();
    assert(msg != nullptr);
    assert(msg->GetArena() == &arena);
    msg->set_name("ArenaProbe");

    uint64_t allocated = arena.SpaceAllocated();
    assert(allocated > 0);

    /* Functionally identical to heap-allocated: serialize works the same way. */
    std::string wire;
    bool ok = fdp->SerializeToString(&wire);
    assert(ok);
    assert(!wire.empty());

    /* DO NOT delete fdp / msg — arena owns them. Arena destructor reclaims. */
    std::printf("protobuf arena test OK: arena_bytes=%llu wire_bytes=%zu\n",
                static_cast<unsigned long long>(allocated), wire.size());

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
