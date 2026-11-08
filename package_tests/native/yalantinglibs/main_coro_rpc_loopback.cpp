/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for yalantinglibs: coro rpc loopback; calls async_simple::coro::syncAwait().
 */

#include <asio/version.hpp>
#include <ylt/coro_rpc/coro_rpc_client.hpp>
#include <ylt/coro_rpc/coro_rpc_server.hpp>

#include <cstdio>
#include <string>

inline int add(int a, int b) { return a + b; }

int main() {
    coro_rpc::coro_rpc_server server(/*thread_num=*/1, /*port=*/0, "127.0.0.1");
    server.register_handler<add>();
    auto started = server.async_start();
    if (started.hasResult()) {
        std::fprintf(stderr, "coro_rpc_server did not start: %s\n", std::string(started.value().message()).c_str());
        return 1;
    }

    coro_rpc::coro_rpc_client client;
    const coro_rpc::err_code connected =
        async_simple::coro::syncAwait(client.connect("localhost", std::to_string(server.port())));
    if (connected) {
        std::fprintf(stderr, "coro_rpc_client could not connect to localhost:%u: %s\n", unsigned(server.port()),
                     std::string(connected.message()).c_str());
        return 1;
    }

    auto result = async_simple::coro::syncAwait(client.call<add>(40, 2));
    if (!result.has_value()) {
        std::fprintf(stderr, "coro_rpc call failed: %s\n", result.error().msg.c_str());
        return 1;
    }
    if (result.value() != 42) {
        std::fprintf(stderr, "coro_rpc call returned %d, expected 42\n", result.value());
        return 1;
    }

    server.stop();
    std::printf("yalantinglibs coro_rpc on asio %d.%d.%d: add(40, 2) = %d over localhost:%u\n", ASIO_VERSION / 100000,
                ASIO_VERSION / 100 % 1000, ASIO_VERSION % 100, result.value(), unsigned(server.port()));
    return 0;
}
