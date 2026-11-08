/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for grpc: core API; calls grpc_init(), grpc_shutdown(), grpc::Version(), grpc::InsecureChannelCredentials().
 */

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

int main() {
    grpc_init();
    grpc_shutdown();

    /* C++ API — version string must be present and non-trivial. */
    const std::string version = grpc::Version();
    assert(!version.empty());
    assert(version.size() >= 3);

    /* Credentials factory — exercises the security registry without any network I/O. */
    std::shared_ptr<grpc::ChannelCredentials> creds = grpc::InsecureChannelCredentials();
    assert(creds != nullptr);

    std::printf("grpc test OK: %s\n", version.c_str());
    return 0;
}
