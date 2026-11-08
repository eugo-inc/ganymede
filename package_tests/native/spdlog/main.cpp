/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for spdlog: core API; compiles against <spdlog/spdlog.h>, <spdlog/sinks/ostream_sink.h>.
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
    /* Capture log output into an in-memory stream — no file I/O. */
    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    auto logger = std::make_shared<spdlog::logger>("demo_smoke", sink);
    logger->set_pattern("[%n] [%l] %v");
    logger->set_level(spdlog::level::info);

    logger->info("hello {} {}", "spdlog", 42);
    logger->flush();

    const std::string captured = oss.str();
    assert(captured.find("demo_smoke") != std::string::npos);
    assert(captured.find("info") != std::string::npos);
    assert(captured.find("hello spdlog 42") != std::string::npos);

    std::printf("spdlog test OK: v%d.%d.%d\n",
                SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    return 0;
}
