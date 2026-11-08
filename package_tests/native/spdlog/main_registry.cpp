/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for spdlog: registry; calls spdlog::drop(), spdlog::get(), spdlog::register_logger().
 *
 * Compiled header-only against an external fmt (-DSPDLOG_FMT_EXTERNAL), so it needs fmt installed
 * and does not exercise libspdlog.so itself.
 */

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <cassert>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

int main(void) {
    const std::string name = "demo_registry_probe";

    spdlog::drop(name);

    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    auto logger = std::make_shared<spdlog::logger>(name, sink);

    /* Not registered yet. */
    assert(spdlog::get(name) == nullptr);

    /* Register. */
    spdlog::register_logger(logger);

    /* Now retrievable, and SAME control block (shared_ptr equality). */
    auto fetched = spdlog::get(name);
    assert(fetched != nullptr);
    assert(fetched.get() == logger.get());

    /* It must actually log when used. */
    fetched->set_pattern("%v");
    fetched->set_level(spdlog::level::info);
    fetched->info("registry_payload");
    fetched->flush();
    assert(oss.str().find("registry_payload") != std::string::npos);

    /* Drop and confirm it's gone. */
    spdlog::drop(name);
    assert(spdlog::get(name) == nullptr);

    std::printf("spdlog registry test OK\n");
    return 0;
}
