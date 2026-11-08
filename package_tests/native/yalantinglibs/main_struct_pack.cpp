/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for yalantinglibs: struct pack; calls struct_pack::serialize(), struct_pack::deserialize().
 */

#include <ylt/struct_pack.hpp>
#include <ylt/version.hpp>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct request {
    std::int64_t id;
    std::string model;
    std::vector<std::int32_t> tokens;
};

int main() {
    const request sent{7, "llama", {1, 2, 3}};
    const std::vector<char> buffer = struct_pack::serialize(sent);

    auto received = struct_pack::deserialize<request>(buffer);
    if (!received) {
        std::fprintf(stderr, "struct_pack::deserialize failed\n");
        return 1;
    }
    if (received->id != sent.id || received->model != sent.model || received->tokens != sent.tokens) {
        std::fprintf(stderr, "struct_pack round trip changed the value\n");
        return 1;
    }

    std::printf("yalantinglibs (YLT_VERSION %d): struct_pack round-tripped %zu bytes\n", YLT_VERSION, buffer.size());
    return 0;
}
