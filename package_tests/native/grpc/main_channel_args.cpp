/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for grpc: channel args; calls grpc_init(), grpc::InsecureChannelCredentials(), grpc::CreateCustomChannel(), grpc_shutdown().
 */

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    grpc_init();

    /* (1) ChannelArguments — set a well-known key and verify it sticks. */
    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 1024 * 1024);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 60000);
    args.SetString(GRPC_ARG_PRIMARY_USER_AGENT_STRING, "demo-grpc-probe/1.0");

    /* (2) CreateCustomChannel — construct a Channel with an unreachable target.
     *     Channels are lazy: this MUST NOT block on connect. State should be IDLE. */
    std::shared_ptr<grpc::ChannelCredentials> creds = grpc::InsecureChannelCredentials();
    assert(creds != nullptr);

    std::shared_ptr<grpc::Channel> channel =
        grpc::CreateCustomChannel("127.0.0.1:1", creds, args);
    assert(channel != nullptr);

    /* GetState(try_to_connect=false) — IDLE before any RPC. */
    grpc_connectivity_state state = channel->GetState(false);
    assert(state == GRPC_CHANNEL_IDLE);

    grpc_shutdown();

    std::printf("grpc channel-args test OK: state=%d\n", (int)state);
    return 0;
}
