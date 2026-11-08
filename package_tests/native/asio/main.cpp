/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for asio: core API; calls asio::post().
 */

#include <asio.hpp>

#include <chrono>
#include <cstddef>
#include <cstdio>

int main() {
    asio::io_context context;
    int ran = 0;

    asio::post(context, [&ran] { ++ran; });

    asio::steady_timer timer(context, std::chrono::milliseconds(1));
    timer.async_wait([&ran](const asio::error_code& error) {
        if (!error) {
            ++ran;
        }
    });

    const std::size_t handlers = context.run();
    if (ran != 2 || handlers != 2) {
        std::fprintf(stderr, "asio: expected 2 handlers to run, got ran=%d run()=%zu\n", ran, handlers);
        return 1;
    }

    std::printf("asio %d.%d.%d: io_context ran %zu handlers\n", ASIO_VERSION / 100000, ASIO_VERSION / 100 % 1000,
                ASIO_VERSION % 100, handlers);
    return 0;
}
