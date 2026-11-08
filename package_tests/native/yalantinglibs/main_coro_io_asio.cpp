/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yalantinglibs: coro io asio; calls coro_io::sleep_for(), coro_io::get_global_executor(), async_simple::coro::syncAwait().
 */

#include <asio/version.hpp>
#include <ylt/coro_io/coro_io.hpp>

#include <cstdio>

#include <chrono>

static async_simple::coro::Lazy<bool> nap(coro_io::ExecutorWrapper<> *executor) {
    co_return co_await coro_io::sleep_for(std::chrono::milliseconds(1), executor);
}

int main() {
    coro_io::ExecutorWrapper<> *executor = coro_io::get_global_executor(1);
    const bool slept = async_simple::coro::syncAwait(nap(executor).via(executor));
    if (!slept) {
        std::fprintf(stderr, "coro_io::sleep_for on the asio timer reported failure\n");
        return 1;
    }

    std::printf("yalantinglibs coro_io on asio %d.%d.%d: 1 ms timer coroutine completed\n", ASIO_VERSION / 100000,
                ASIO_VERSION / 100 % 1000, ASIO_VERSION % 100);
    return 0;
}
